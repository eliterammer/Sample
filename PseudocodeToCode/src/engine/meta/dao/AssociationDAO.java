package engine.meta.dao;

import java.io.IOException;
import java.util.HashSet;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.Set;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hbase.HBaseConfiguration;
import org.apache.hadoop.hbase.client.Get;
import org.apache.hadoop.hbase.client.HTable;
import org.apache.hadoop.hbase.client.Put;
import org.apache.hadoop.hbase.client.Result;
import org.apache.hadoop.hbase.util.Bytes;

import engine.meta.Association;

public class AssociationDAO {

	
	private Configuration config;
	private HTable wordCommandAssocTable=null;
	private void getConnection() throws IOException{
		if(wordCommandAssocTable == null){
			 config = HBaseConfiguration.create();
			 config.set("hbase.zookeeper.quorum", "localhost");  // Here we are running zookeeper locally
			 wordCommandAssocTable = new HTable(config, "word_command_assoc");
		}
	}
	public void addSimilarityInformation(String wordA,String wordB,int similarity) throws IOException {
		getConnection();
		HTable table = new HTable(config, "word_similarity");
		Put p = new Put(Bytes.toBytes(wordA));
		p.add(Bytes.toBytes("S"), Bytes.toBytes(wordB),Bytes.toBytes(similarity));
		table.put(p);	
		p = new Put(Bytes.toBytes(wordB));
		p.add(Bytes.toBytes("S"), Bytes.toBytes(wordA),Bytes.toBytes(similarity));
		table.put(p);
	}

	public String getAssociation(String word,String POS) throws IOException {
		System.out.println(POS);
		getConnection();
		System.out.println(word+POS);
		Get g = new Get(Bytes.toBytes(word));
		Result r = wordCommandAssocTable.get(g);
		byte[] value = r.getValue(Bytes.toBytes("POS"), Bytes.toBytes(POS));
		return Bytes.toString(value);
	}
	
	public int getKnownSimilarity(String wordA,String wordB) throws IOException {
		getConnection();
		System.out.println("check ||| btwn "+wordA+" "+wordB);
		Get g = new Get(Bytes.toBytes(wordA));
		HTable table = new HTable(config, "word_similarity");
		Result r = table.get(g);
		byte[] value = r.getValue(Bytes.toBytes("S"), Bytes.toBytes(wordB));
		System.out.println("knwown ||| "+Bytes.toString(value));
		if(value == null){
			return -1;
		}
		return Bytes.toInt(value);
	}
	
	public Set<String> getKnownSimilarWords(String wordA) throws IOException {
		System.out.println("known similar words "+wordA);
		getConnection();
		Set<String> returnValue = new HashSet<String>();
		Get g = new Get(Bytes.toBytes(wordA));
		HTable table = new HTable(config, "word_similarity");
		Result r = table.get(g);
		NavigableMap<byte[],byte[]> columnsMap = r.getFamilyMap(Bytes.toBytes("S"));
		if(columnsMap == null){
			return null;
		}
		for(Entry<byte[], byte[]> entry :columnsMap.entrySet()){
			returnValue.add(Bytes.toString(entry.getKey()));
		}
	//	byte[] value = r.getValue(Bytes.toBytes("S"), Bytes.toBytes(wordA));
		return returnValue;
	}
}
