package engine.meta;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.ObjectInputStream;

import engine.meta.classifier.BayesianClassifier;

public class KnowledgeManager {

	private static KnowledgeManager manager = null;
	private KnowledgeManager(){
	}
	public void fetch(String command) throws Exception{
		ObjectInputStream ois = null;
		BayesianClassifier knowledge = null;
		try{
			ois = new ObjectInputStream(new FileInputStream(command+".know"));
			
		}catch(FileNotFoundException fne){
			//knowledge =  new Ba
		}
		knowledge = (BayesianClassifier) ois.readObject();
		System.out.println(knowledge);
	}
	public static KnowledgeManager getInstance(){
		if(manager == null){
			return new KnowledgeManager();
		}
		return manager;
	}

	
}
