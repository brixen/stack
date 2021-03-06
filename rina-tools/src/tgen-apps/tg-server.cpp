#include <chrono>
#include <map>

#include <tclap/CmdLine.h>

#include "ra_base_server.h"
#include "test_commons.h"

struct flow_log {
	flow_log() {
		count = 0;
		data = 0;
		maxLat = 0;
		minLat = 9999999999;
		latCount = 0;
		seq_id = 0;
		flowId = 0;
		QoSId = 0;
	}

	void process(long long t, dataSDU * sdu) {
		seq_id = sdu->SeqId;
		count++;
		data += sdu->Size;
		long long lat = t - sdu->SendTime;
		if (lat > maxLat) maxLat = lat;
		if (lat < minLat) minLat = lat;
		latCount += lat;
	}

	int QoSId;
	int flowId;

	int seq_id;
	long long count, data;

	long long maxLat;
	long long minLat;
	long double latCount;
};

struct qos_log {
	qos_log() {
		count = 0;
		data = 0;
		total = 0;
		maxLat = 0;
		minLat = 9999999999;
		latCount = 0;
	}
	void process(flow_log * f) {
		count += f->count;
		total += f->seq_id + 1;
		data += f->data;
		if (f->maxLat > maxLat) maxLat = f->maxLat;
		if (f->minLat < minLat) minLat = f->minLat;
		latCount += f->latCount;
	}

	long long count, total, data;

	long long maxLat;
	long long minLat;
	long double latCount;
};

class TgServer : public ra::BaseServer {
public:
	TgServer(const std::string& Name, const std::string& Instance,
		  bool verbose, const std::string& type) :
		BaseServer(Name, Instance, verbose) {
		RegisterTimeoutMs = 1000;
		ListenTimeoutMs = -1;
		Count = 0;
		server_type = type;
	}

	virtual ~TgServer(){};

protected:

	int HandleFlow(const int Fd) {
		if (server_type == "log") {
			return HandleFlow_log(Fd);
		}

		if (server_type == "drop") {
			return HandleFlow_drop(Fd);
		}

		if (server_type == "dump") {
			return HandleFlow_dump(Fd);
		}

		return -1;
	}

private:
	std::string server_type;
	int Count;
	std::vector<flow_log*> flow_logs;
	std::map<int, qos_log> qos_logs;
	std::mutex Mt;
	union {
		ra::byte_t Buffer[BUFF_SIZE];
		dataSDU Data;
		initSDU InitData;
	};

	void logger_t() {
		int fCount;
		std::chrono::seconds s5(5);

		do {
			std::this_thread::sleep_for(s5);
			Mt.lock();
			fCount = Count;
			Mt.unlock();
			if (verbose)
				std::cout << "Flows in progress " << fCount << std::endl;
		} while (fCount > 0);

		Mt.lock();

		if (verbose)
			std::cout << "Currently no flows in progress, print log:" << std::endl << std::endl;

		//Process log
		long long tCount = 0, tData = 0;

		for (auto f : flow_logs) {
			tCount += f->count;
			tData += f->data;
			qos_logs[f->QoSId].process(f);
		}

		std::cout << "Total :: " << tCount << " | " << (tData / 125000.0) << " Mb" << std::endl;

		for (flow_log * f : flow_logs) {
			long long c = f->count;
			long long t = f->seq_id;
			long double l = t - c;

			std::cout << "\t" << f->flowId << " (" << (int)f->QoSId << ") | "
				<< c << " / " << t << " (" << (l*100.0 / c) << " %) | " << f->data << " B"
				<< " ||" << (f->minLat / 1000.0)
				<< " -- " << (f->latCount / (1000.0*c))
				<< " -- " << (f->maxLat / 1000.0)
				<< std::endl;
		}

		for (auto qd : qos_logs) {
			int QoSId = qd.first;
			qos_log & q = qd.second;

			long long l = q.total - q.count;

			std::cout << "\t(" << QoSId << ") | "
				<< q.count << " | " << l << " | " << q.total << "  || "
				<< (q.minLat / 1000.0) << "  -- " << (q.latCount / (1000 * q.count))<< "  -- " << (q.maxLat / 1000.0);
			std::cout << "\t(" << QoSId << ")\t"
				<< (100.0*l / q.total) << " % || "
				<< (q.latCount / (1000 * q.count)) << " -- " << (q.maxLat / 1000.0) << std::endl;

		}

		for (flow_log * f : flow_logs) {
			delete f;
		}

		Count = 0;
		flow_logs.clear();
		Mt.unlock();
	}

	int process_first_sdu(const int Fd) {
		if (ra::ReadDataTimeout(Fd, Buffer, 1000) <= 0) {
			std::cerr << "No data received during the first second of lifetime" << std::endl;
			return -1;
		}

		if (InitData.Flags & SDU_FLAG_INIT == 0) {
			std::cerr << "First received packet not with INIT flag" << std::endl;
			return -1;
		}

		if (InitData.Flags & SDU_FLAG_NAME) {
			if (verbose)
				std::cout << "Started Flow " << Fd << " -> "
				<< (Buffer + sizeof(initSDU)) << std::endl;
		} else {
			if (verbose)
				std::cout << "Started Flow " << Fd << std::endl;
		}

		if (write(Fd, Buffer, InitData.Size) != (int)InitData.Size) {
			std::cerr << "First packet ECHO failed" << std::endl;
			return -1;
		}

		return 0;
	}

	int HandleFlow_log(const int Fd) {
		int result, ReadSize;
		bool start_logger;

		result = process_first_sdu(Fd);
		if (result != 0)
			return result;

		flow_log * Flow = new flow_log();
		Flow->QoSId = InitData.QoSId;
		Flow->flowId = InitData.FlowId;
		Flow->seq_id = Data.SeqId;

		Mt.lock();
		start_logger = (Count <= 0);
		Count++;
		flow_logs.push_back(Flow);
		Mt.unlock();

		if (start_logger) {
			std::thread t(&TgServer::logger_t, this);
			t.detach();
		}

		for (;;) {
			ReadSize = ra::ReadData(Fd, Buffer);

			Mt.lock();

			if (ReadSize <= 0) {
				Count--;
				Mt.unlock();
				return -1;
			}

			if (Data.Flags & SDU_FLAG_FIN) {
				Count--;
				Mt.unlock();
				break;
			}

			Flow->process(std::chrono::duration_cast<std::chrono::milliseconds>(
							std::chrono::system_clock::now().time_since_epoch()).count(),
							&Data);
			Mt.unlock();

		}

		if (write(Fd, Buffer, Data.Size) != (int) Data.Size) {
			std::cerr << "Last packet ECHO failed" << std::endl;
			return -1;
		}
		return 0;
	}

	int HandleFlow_drop(const int Fd) {
		int result, ReadSize;

		result = process_first_sdu(Fd);
		if (result != 0)
			return result;

		for (;;) {
			ReadSize = ra::ReadData(Fd, Buffer);
			if (ReadSize <= 0) {
				return -1;
			}
			if (Data.Flags & SDU_FLAG_FIN) {
				if (Data.Flags & SDU_FLAG_NAME) {
					if (verbose)
						std::cout << "Ended Flow " << Fd << " -> "
							  << (Buffer + sizeof(dataSDU)) << std::endl;
				} else {
					if (verbose)
						std::cout << "Ended Flow " << Fd << std::endl;
				}
				break;
			}
		}

		if (write(Fd, Buffer, Data.Size) != (int) Data.Size) {
			std::cerr << "Last packet ECHO failed" << std::endl;
			return -1;
		}

		return 0;
	}

	int HandleFlow_dump(const int Fd) {
		long long InitTime;
		int result, ReadSize;

		result = process_first_sdu(Fd);
		if (result != 0)
			return result;

		for (;;) {
			if(ra::ReadData(Fd, Buffer) <= 0) { return -1;}

			if (Data.Flags & SDU_FLAG_FIN) {
				break;
			}

			std::cout << Fd<< " | " << Data.SeqId  << " | " << Data.Size
				  << "B | " << (Data.SendTime - InitTime) << std::endl;
		}

		if (write(Fd, Buffer, Data.Size) != (int) Data.Size) {
			std::cerr << "Last packet ECHO failed" << std::endl;
			return -1;
		}

		return 0;
	}
};


int main(int argc, char ** argv) {
	std::string Name, Instance, type;
	std::vector<std::string> DIFs;
	bool verb;

	try {
		TCLAP::CmdLine cmd("LogServer", ' ', "2.0");

		TCLAP::ValueArg<std::string> Name_a("n","name","Application process name, default = TgServer", false, "TgServer", "string");
		TCLAP::ValueArg<std::string> Instance_a("i","instance","Application process instance, default = 1", false, "1", "string");
		TCLAP::ValueArg<std::string> type_a("t","type","Server type (options = log, dump, drop), default = log", false, "log", "string");
		TCLAP::UnlabeledMultiArg<std::string> DIFs_a("difs","DIFs to use, empty for any DIF", false, "string");
		TCLAP::SwitchArg verbose("v", "verbose", "display more debug output, default = false",false);

		cmd.add(Name_a);
		cmd.add(Instance_a);
		cmd.add(DIFs_a);
		cmd.add(verbose);
		cmd.add(type_a);

		cmd.parse(argc, argv);

		Name = Name_a.getValue();
		Instance = Instance_a.getValue();
		DIFs = DIFs_a.getValue();
		verb = verbose.getValue();
		type = type_a.getValue();
	}
	catch (TCLAP::ArgException &e) {
		std::cerr << e.error() << " for arg " << e.argId() << std::endl;
		std::cerr << "Failure reading parameters." << std::endl;
		return -1;
	}

	TgServer App(Name, Instance, verb, type);
	for (auto DIF : DIFs) {
		App.AddToDIF(DIF);
	}

	return App.Run();
}
