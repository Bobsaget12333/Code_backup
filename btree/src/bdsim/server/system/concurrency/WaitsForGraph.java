package bdsim.server.system.concurrency;

import java.util.Set;

import bdsim.server.system.BDSystemThread;

/**
 * A graph of lock dependencies to detect cycles.
 * 
 * @author wpijewsk, dclee
 */
public class WaitsForGraph {
	
	/**
	 * Builds a graph by analyzing a set of locks. You should use this in
	 * production code.
	 * 
	 * @param locks  The locks currently held in the system.
	 */
	public WaitsForGraph(Set<BDTrackableReadWriteLock<BDSystemThread>> locks) {
	}

	/** 
	 * @return  Whether or not this graph has a cycle
	 */
	public boolean hasCycle() {		
		return false;
	}

	/**
	 * @return  A thread which will break the cycle of dependencies
	 */
	public int getCycleBreaker() {
		return 0;
	}

  public static void main(String[] args) {
    //TODO: Fill in for testing your graph cycles (OPTIONAL)
  }
}
