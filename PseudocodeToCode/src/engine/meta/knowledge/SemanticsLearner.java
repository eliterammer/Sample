package engine.meta.knowledge;


import engine.meta.GrammarParser;

public class SemanticsLearner {
	
	public int addKnowledge(String line) throws Exception{
		String ruleName = new GrammarParser("grammar.xml").findRuleFromGrammar(line);
		return 0;
	}

}
