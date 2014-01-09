/*
 *Note: This is just a test code for a distributed crypto project I am
 *working on as a hobby.  Must fully test the functionality, then will optimize!
 */

import java.rmi.Remote;
import java.rmi.RemoteException;

public interface Hello extends Remote {
    String sayHello(String filename) throws RemoteException;
}