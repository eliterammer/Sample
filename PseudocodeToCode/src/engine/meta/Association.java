package engine.meta;

import java.util.HashMap;
import java.util.Map;

public class Association {

	private String word;
	private Map<String,Map<String,Integer>> mapOfassociations;
	private String mostLikely;
	
	public Association(){
		mapOfassociations = new HashMap<String,Map<String,Integer>>();
	}
	public String getWord() {
		return word;
	}
	public void setWord(String word) {
		this.word = word;
	}
	public Map<String,Map<String,Integer>> getAssociations() {
		return mapOfassociations;
	}
	public String getMostLikely() {
		return mostLikely;
	}
}
