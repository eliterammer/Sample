package test.engine.meta.knowledge;


import org.junit.Test;

import engine.meta.knowledge.SemanticsLearner;

public class SemanticsLearnerTest {

	@Test
	public void shouldLocateTheRightProduction() throws Exception{
		SemanticsLearner sl = new SemanticsLearner();
		sl.addKnowledge("scanf(\"%d\",a");
	}
}
