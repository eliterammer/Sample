package test.engine.main;

import java.util.ArrayList;

import org.junit.Assert;
import org.junit.Test;

import engine.main.AssociationsManager;


public class AssociationsManagerTest {

	private static 	AssociationsManager as = new AssociationsManager();

	@Test
	public void objectShouldBeCreated(){
		AssociationsManager as = new AssociationsManager();
	}
	
	@Test
	public void shouldAnalyseBasicSentenceCorrectly_1() throws Exception{
		ArrayList<String> context = new ArrayList<String>(5);
		context.add("READ");
		Assert.assertEquals("READ",as.findConstruct("read an integer",context));
	}
	//@Test
/*	public void shouldAnalyseIncompleteSentenceCorrectly_1() throws Exception{
		ArrayList<String> context = new ArrayList<String>(5);
		context.add("PROCEDURE");
		Assert.assertEquals(as.findConstruct("new procedure bubbleSort",context), "READ");
	}*/
}
