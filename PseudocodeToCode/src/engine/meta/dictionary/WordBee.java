package engine.meta.dictionary;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.http.client.utils.URLEncodedUtils;
import org.apache.log4j.Logger;

import engine.meta.dao.AssociationDAO;
import engine.meta.dao.DictionaryDAO;

public class WordBee {
	private static Logger logger = Logger.getLogger(WordBee.class);
	private static StringBuilder getContents(String word,String POS) throws Exception{
		StringBuilder contents =  null;
		try{
			contents = getContentsFromThesaurus(word, POS);
		}catch(IOException fne){
			try{
				contents = getContentsFromThesaurus(word, null);
			}catch(IOException fneInner){
				System.out.println("no such word in dictionary");
			}
		}
		return contents;
	}
	
	private static Set<String> getSynonymsFromThesaurus(String word,String POS) throws Exception{
		DictionaryDAO dDao = new DictionaryDAO();
		Set<String> results=null;
		results = dDao.getWordCloud(word);
		if(results != null){
			return results;
		}
		AssociationDAO dao = new AssociationDAO();
		StringBuilder contents =  getContents(word, POS);
		if(contents == null){
			Set<String> similarWords =  dao.getKnownSimilarWords(word);
			if(similarWords == null){
				return null;
			}
			for(String synonym:similarWords){
				contents =  getContents(synonym, POS);
			}
		}
		results= extractWordCloud(contents,POS);
		logger.debug("extarcted word cloud from M-W "+results);
		logger.debug("addint to db");
		dDao.addWordCloud(word,results);
		return results;
	}

	private static Set<String> extractWordCloud(StringBuilder contents,String POS) throws IOException {
		Set<String> wordCloud= new HashSet<String>(50);
		Set<String> wordSet = extractSynonyms(contents);
		for(String word:wordSet){
			if(word.contains(" ")){
				continue;
			}
			StringBuilder innerContents = null;
			try{
			 innerContents =  getContentsFromThesaurus(word, POS);
			}catch(FileNotFoundException fne){
				try{
					innerContents =  getContentsFromThesaurus(word, null);
				}catch(FileNotFoundException ifne){
					System.out.println(ifne.getMessage());//ifne.printStackTrace();	
					continue;
				}
			}
			wordCloud.add(word);
			wordCloud.addAll(extractSynonyms(innerContents));
		}
		return wordCloud;
	}

	private static Set<String> extractSynonyms(StringBuilder contents) {
		int lower=0,upper=0;
		int limit = contents.indexOf("Near Antonyms");
		upper= contents.indexOf("Related Words");
		Set<String> wordSet = new HashSet<String>(10); 
		while (lower != -1) {
			lower = contents.indexOf("<a", upper);
			lower = contents.indexOf(">", lower);
			lower += 1;
			upper = contents.indexOf("</a>", lower);
			if(upper > limit){
				break;
			}
			wordSet.add(contents.substring(lower,upper));
		}
		return wordSet;
	}

	private static StringBuilder getContentsFromThesaurus(String word, String POS)
			throws MalformedURLException, IOException {
		String transformedPOS=null;
		String encodedURL = null;

		if(POS == null){
			encodedURL = "http://www.merriam-webster.com/thesaurus/"+word;
		}else if(POS.equals("VB")){
			transformedPOS="verb";
			encodedURL = "http://www.merriam-webster.com/thesaurus/"+word+URLEncoder.encode("["+transformedPOS+"]","UTF-8");
		}else{
			encodedURL = "http://www.merriam-webster.com/thesaurus/"+word+URLEncoder.encode("["+POS+"]","UTF-8");
		}
		URL url =  new URL(encodedURL);
		BufferedReader br = new BufferedReader(new InputStreamReader(url.openStream()));
		StringBuilder contents = new StringBuilder(500);
		char buffer[] = new char[4096];
		int l=0;
		while((l = br.read(buffer)) != -1){
			contents.append(buffer,0,l);
		}
		return contents;
	}

	public static int getSimilarityMeasure(String wordA,String wordB,String POS) throws Exception{
		AssociationDAO dao = new AssociationDAO();
		int knownSimilarity = dao.getKnownSimilarity(wordA, wordB);
		System.out.println(knownSimilarity);
		if(knownSimilarity != -1){
			return knownSimilarity;
		}
		int score = 0;
		if(wordA.contains(wordB) ||  wordB.contains(wordA)){
			score=3;
			score+=5*Math.pow(2,-Math.abs(wordA.length()-wordB.length()));
			//return score;
		}else{
			score = getSimilarityBasedOnWordClouds(wordA,wordB,POS);
		}
		if(score > 0){
			dao.addSimilarityInformation(wordA, wordB, score);
		}
		return score;
	}

	private static int getSimilarityBasedOnWordClouds(String wordA, String wordB, String POS) throws Exception{
		Set<String> wordACloud = getSynonymsFromThesaurus(wordA, POS);
		Set<String> wordBCloud = getSynonymsFromThesaurus(wordB, POS);
		int count = 0;
		for(String word:wordACloud){
			if(wordBCloud.contains(word)){
				count+=1;
			}
		}
		
		return count;
	}
}
