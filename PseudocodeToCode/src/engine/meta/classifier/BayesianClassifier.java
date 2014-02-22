package engine.meta.classifier;

import java.io.Serializable;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class BayesianClassifier implements Serializable{

	private Map<String,Long> occurenceCounts=null;
	private Map<String,Map<String,Long>> likelyhoodMap=null;

	private long totalNumber=0L;
	private List<Integer> hints = null;
	private String delim = null;
	public BayesianClassifier(List<Integer> hints,String delim){
		occurenceCounts = new HashMap<String,Long>();
		likelyhoodMap = new HashMap<String,Map<String,Long>>();
		this.hints= hints;
		this.delim = delim;
	}
	
	public void addInformation(String input){
		totalNumber++;
		String[] fields = input.split(delim);
		for(int i =0;i<fields.length-1;i++){
			if(hints.contains(i)){
				Map<String,Long> likelyhood =null;
				if(occurenceCounts.containsKey(fields[fields.length-1])){
					likelyhood = likelyhoodMap.get(fields[fields.length-1]);
					updateCountInMap(likelyhood,fields[i]);
				}else{
					likelyhood = new HashMap<String,Long>();
					likelyhood.put(fields[i],1L);
				}
				likelyhoodMap.put(fields[fields.length-1], likelyhood);
				updateCountInMap(occurenceCounts,fields[fields.length-1]);
			}
		}
	}
	private void updateCountInMap(Map<String, Long> mapOfCounts,String key) {
		long count = 1;
		if(mapOfCounts.containsKey(key)){
			count = mapOfCounts.get(key);
			count++;
		}
		mapOfCounts.put(key,count);
	}

	public long getTotalCount(){
		return totalNumber;
	}
	public String classify(String input){
		System.out.println(occurenceCounts);
		System.out.println(likelyhoodMap);
		String[] fields = input.split(delim);
		double max = 0;
		String classification=null;
		for(String C:occurenceCounts.keySet()){
			double posterior = calculatePosterior(fields, C);
			if(posterior >= max){
				max = posterior;
				classification = C;
			}
		}
		return classification;
	}

	private double calculatePosterior(String[] fields, String C) {
		long countOfC=0;
		double posterior =1;
		Map<String,Long> likelyhood = null;
		if(occurenceCounts.containsKey(C)){
			countOfC =occurenceCounts.get(C);
			likelyhood = likelyhoodMap.get(C);
			for(int i =0;i<fields.length;i++){
				// add Laplace correction(?)
				if(hints.contains(i)){
					if(likelyhood.containsKey(fields[i])){
						long count = likelyhood.get(fields[i]);
						posterior*=count/countOfC;
					}else{
						posterior=0;
					}
				}
			}
		}
		return posterior;
	}
	public String toString(){
		return occurenceCounts.toString();
	}
}
