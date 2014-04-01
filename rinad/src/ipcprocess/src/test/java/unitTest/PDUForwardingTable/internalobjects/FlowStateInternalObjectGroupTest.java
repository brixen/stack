package unitTest.PDUForwardingTable.internalobjects;


import static org.junit.Assert.*;

import org.junit.Test;

import rina.encoding.impl.googleprotobuf.flowstate.FlowStateGroupEncoder;
import rina.ipcprocess.impl.PDUForwardingTable.PDUFTImpl;
import rina.ipcprocess.impl.PDUForwardingTable.internalobjects.FlowStateInternalObject;
import rina.ipcprocess.impl.PDUForwardingTable.internalobjects.FlowStateInternalObjectGroup;
import rina.ipcprocess.impl.PDUForwardingTable.ribobjects.FlowStateRIBObjectGroup;
import rina.ribdaemon.api.RIBDaemonException;
import unitTest.PDUForwardingTable.fakeobjects.FakeCDAPSessionManager;

public class FlowStateInternalObjectGroupTest {
	
/*
	@Test
	public void add_AddObject_Added()
	{
		long address = 1;
		int portId = 1;
		long neighborAddress = 2;
		int neigborPortID = 1;
		boolean state = true;
		int sequenceNumber = 1;
		int age = 1;
		FlowStateInternalObject o1 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, state, sequenceNumber, age);
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		
		g1.add(o1);
		
		assertEquals(g1.getFlowStateObjectArray().get(0), o1);
	}
	
	@Test
	public void FlowStateInternalObjectGroup_ParameterConstructor_Constructed()
	{
		long address = 1;
		int portId = 1;
		long neighborAddress = 2;
		int neigborPortID = 1;
		boolean state = true;
		int sequenceNumber = 1;
		int age = 1;
		FlowStateInternalObject o1 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, state, sequenceNumber, age);
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		
		g1.add(o1);
		FlowStateInternalObjectGroup g2 = new FlowStateInternalObjectGroup(g1.getFlowStateObjectArray());
		
		assertEquals(g1.getFlowStateObjectArray(), g2.getFlowStateObjectArray());
	}
	
	@Test
	public void getByPortId_NoObjects_Null()
	{
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		
		assertEquals(g1.getByPortId(1), null);
	}
	
	@Test
	public void getByPortId_RetrieveObjectAdded_EqualObjectAdded()
	{
		long address = 1;
		int portId = 1;
		long neighborAddress = 2;
		int neigborPortID = 1;
		boolean state = true;
		int sequenceNumber = 1;
		int age = 1;
		FlowStateInternalObject o1 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, state, sequenceNumber, age);
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		
		g1.add(o1);
		
		assertEquals(g1.getByPortId(portId), o1);
	}
	
	@Test
	public void getByPortId_ObjectNotFound_Null()
	{
		long address = 1;
		int portId = 1;
		long neighborAddress = 2;
		int neigborPortID = 1;
		boolean state = true;
		int sequenceNumber = 1;
		int age = 1;
		FlowStateInternalObject o1 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, state, sequenceNumber, age);
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		
		g1.add(o1);
		
		assertEquals(g1.getByPortId(2), null);
	}
	
	@Test
	public void getModifiedFSO_ObjectAdded_Object()
	{
		long address = 1;
		int portId = 1;
		long neighborAddress = 2;
		int neigborPortID = 1;
		boolean state = true;
		int sequenceNumber = 1;
		int age = 1;
		FlowStateInternalObject o1 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, state, sequenceNumber, age);
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		
		g1.add(o1);
		
		assertEquals(g1.getModifiedFSO().get(0), o1);
	}
	
	@Test
	public void getModifiedFSO_ObjectsAdded_Objects()
	{
		long address = 1;
		int portId = 1;
		long neighborAddress = 2;
		int neigborPortID = 1;
		boolean state = true;
		int sequenceNumber = 1;
		int age = 1;
		FlowStateInternalObject o1 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, state, sequenceNumber, age);
		FlowStateInternalObject o2 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, state, sequenceNumber, age);
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		
		g1.add(o1);
		g1.add(o2);
		
		assertEquals(g1.getModifiedFSO().get(0), o1);
		assertEquals(g1.getModifiedFSO().get(1), o2);
	}
	
	@Test
	public void getModifiedFSO_2Objects1notModified_ObjectModified()
	{
		long address = 1;
		int portId = 1;
		long neighborAddress = 2;
		int neigborPortID = 1;
		boolean state = true;
		int sequenceNumber = 1;
		int age = 1;
		FlowStateInternalObject o1 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, state, sequenceNumber, age);
		FlowStateInternalObject o2 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, state, sequenceNumber, age);
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		
		o1.setModified(false);
		g1.add(o1);
		g1.add(o2);
		
		assertEquals(g1.getModifiedFSO().get(0), o2);
	}
	
	@Test
	public void getModifiedFSO_NoObjects_null()
	{
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		
		assertEquals(g1.getModifiedFSO(), null);
	}
	
	@Test
	public void getModifiedFSO_ObjectnotModified_null()
	{
		long address = 1;
		int portId = 1;
		long neighborAddress = 2;
		int neigborPortID = 1;
		boolean state = true;
		int sequenceNumber = 1;
		int age = 1;
		FlowStateInternalObject o1 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, state, sequenceNumber, age);
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		
		o1.setModified(false);
		g1.add(o1);
		
		assertEquals(g1.getModifiedFSO(), null);
	}
	

	
	@Test
	public void incrementAge_2Objects_Incremented()
	{
		long address = 1;
		int portId = 1;
		long neighborAddress = 2;
		int neigborPortID = 1;
		FlowStateInternalObject o1 = new FlowStateInternalObject(address, portId, neighborAddress, neigborPortID, true, 1, 1);
		FlowStateInternalObject o2 = new FlowStateInternalObject(neighborAddress, neigborPortID, address, portId, true, 1, 30);
		FlowStateInternalObjectGroup g1 = new FlowStateInternalObjectGroup();
		PDUFTImpl impl = new PDUFTImpl(2147483647);
		FakeRIBDaemon ribD =new FakeRIBDaemon();
		FakeIPCProcess ipcp = new FakeIPCProcess(new FakeCDAPSessionManager(), ribD , new FlowStateGroupEncoder());
		impl.setIPCProcess(ipcp);
		FlowStateRIBObjectGroup fsRIBGroup = new FlowStateRIBObjectGroup(impl, ipcp);
		try 
		{
			ribD.addRIBObject(fsRIBGroup);
			g1.add(o1,fsRIBGroup);
			g1.add(o2, fsRIBGroup);
			g1.incrementAge(200, fsRIBGroup, impl.getDB());
		} catch (RIBDaemonException e) {
			e.printStackTrace();
		}
		
		assertEquals(g1.getFlowStateObjectArray().get(0).getAge(), 2);
		assertEquals(g1.getFlowStateObjectArray().get(1).getAge(), 31);
	}
	*/
}