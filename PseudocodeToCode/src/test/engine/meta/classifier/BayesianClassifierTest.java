package test.engine.meta.classifier;

import java.util.ArrayList;
import java.util.List;

import org.junit.Assert;
import org.junit.Test;

import engine.meta.classifier.BayesianClassifier;

public class BayesianClassifierTest {

	@Test
	public void checkObjectCreation(){
		new BayesianClassifier(null,null);
	}
	
	@Test
	public void addInfoShouldWork_1(){
		List<Integer> hints = new ArrayList<Integer>(1);
		hints.add(0);
		BayesianClassifier bc = new BayesianClassifier(hints,",");
		bc.addInformation("int,%d");
		Assert.assertEquals("{%d=1}",bc.toString());
		Assert.assertEquals(1, bc.getTotalCount());
	}
	private BayesianClassifier buildClassifier() {
		List<Integer> hints = new ArrayList<Integer>(1);
		hints.add(0);
		BayesianClassifier bc = new BayesianClassifier(hints,",");
		bc.addInformation("int,%d");
		bc.addInformation("int,%d");
		bc.addInformation("int,%d");
		bc.addInformation("float,%f");
		return bc;
	}
	
	@Test
	public void addInfoShouldWork_2(){
		BayesianClassifier bc = buildClassifier();
		Assert.assertEquals("{%f=1, %d=3}",bc.toString());
		Assert.assertEquals(4, bc.getTotalCount());
	}

	@Test
	public void shouldClassify_1(){
		List<Integer> hints = new ArrayList<Integer>(1);
		hints.add(0);
		BayesianClassifier bc = buildClassifier();
		Assert.assertEquals("%d", bc.classify("int"));
		Assert.assertEquals("%f", bc.classify("float"));
	}
	
	//// complex test case
}
