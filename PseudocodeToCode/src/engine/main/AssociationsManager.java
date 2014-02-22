package engine.main;

import java.io.File;
import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;

import engine.meta.Production;
import engine.meta.dao.AssociationDAO;
import engine.meta.dictionary.WordBee;
import opennlp.tools.cmdline.postag.POSModelLoader;
import opennlp.tools.postag.POSModel;
import opennlp.tools.postag.POSSample;
import opennlp.tools.postag.POSTaggerME;
import opennlp.tools.tokenize.WhitespaceTokenizer;
import opennlp.tools.util.ObjectStream;
import opennlp.tools.util.PlainTextByLineStream;

public class AssociationsManager {

	private static Logger logger = Logger.getLogger(AssociationsManager.class);
	private POSTaggerME tagger;
	private AssociationDAO assocDAO=null;
	private String[] context = null;
	private String currentVariable = null;
	private int counter=1;
	public AssociationsManager(){
	POSModel model = new POSModelLoader().load(new File("en-pos-maxent.bin"));
	tagger = new POSTaggerME(model);
	assocDAO = new AssociationDAO();
	}
	
	public String findConstruct(String input,List<String> context) throws Exception{
		ObjectStream<String> lineStream = new PlainTextByLineStream(new StringReader(input));
		String line=null;
		if((line = lineStream.read()) != null) {
			String whitespaceTokenizerLine[] = WhitespaceTokenizer.INSTANCE.tokenize(line);
			String[] tags = tagger.tag(whitespaceTokenizerLine);	
			String construct = findBestConstruct(context, whitespaceTokenizerLine, tags);
			POSSample taggedLine = new POSSample(whitespaceTokenizerLine, tags);
			System.out.println(taggedLine);
			return construct;
		}
		return null;
	}

	private String findBestConstruct(List<String> context,String[] tokens, String[] tags) throws Exception {
		// imperative sentences
		this.context=tokens;
		System.out.println(Arrays.asList(tags));
		if(tags[0].equals("VB") && tags[tags.length-1].equals("NN")){
			System.out.println(tags[1] + tokens[1]);
			if(tags[1].equals("DT") && (tokens[1].equalsIgnoreCase("an") || tokens[1].equalsIgnoreCase("a"))){
				currentVariable = "temp"+counter;
			}
			String machedCommand = pickWorkWithHighestSimilarityMeasure(context.toArray(new String[context.size()]),this.context[0],"VB");
			if(context.contains(machedCommand)){
				return assocDAO.getAssociation(machedCommand, "VERB");
			}
		}
		return null;
	}
	
	public String getIdFromContext(){
		return currentVariable;
	}

	public String getBestComponentChoice(Production production, String component) throws Exception {
		String returnValue=null;
		returnValue = checkForBaseCases(production, component);
		if(returnValue != null){
			return returnValue;
		}
		String[] possibilities = production.value.split("\\|");
		return pickWorkWithHighestSimilarityMeasure(possibilities,context[2],"noun");
	}

	private String pickWorkWithHighestSimilarityMeasure(String[] possibilities,String meaning,String POS) throws Exception {
		String returnValue=null;
		int maxMatchValue=0;
		for(String part:possibilities){
			int similarity = WordBee.getSimilarityMeasure(part, meaning,POS);
			if(maxMatchValue < similarity){
				maxMatchValue=similarity;
				returnValue=part;
			}
		}
		return returnValue;
	}

	private String checkForBaseCases(Production production, String component) {
		if(component.equals("sp;")){
			return " ";
		}
		if(component.equals("ID")){
			return currentVariable;
		}
		if(production.isToken){
			return production.name;
		}
		if(!production.value.contains("|")){
			return production.value;
		}
		return null;
	}
}
