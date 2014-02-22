package engine.meta.dao;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.Set;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hbase.HBaseConfiguration;
import org.apache.hadoop.hbase.client.Get;
import org.apache.hadoop.hbase.client.HTable;
import org.apache.hadoop.hbase.client.Put;
import org.apache.hadoop.hbase.client.Result;
import org.apache.hadoop.hbase.util.Bytes;
import org.apache.log4j.Logger;

public class DictionaryDAO {
	private static Logger logger = Logger.getLogger(DictionaryDAO.class);
	private Configuration config;
	private HTable wordCloudTable=null;
	private void getConnection() throws IOException{
		if(wordCloudTable == null){
			 config = HBaseConfiguration.create();
			 config.set("hbase.zookeeper.quorum", "localhost");  // Here we are running zookeeper locally
			 wordCloudTable = new HTable(config, "word_cloud");
		}
	}
	public void addWordCloud(String word,Set<String> wordCloud) throws IOException {
		logger.debug("adding wordCloud");
		logger.debug("word ---"+wordCloud);
		getConnection();
		HTable table = new HTable(config, "word_cloud");
		Put p = new Put(Bytes.toBytes(word));
		ByteArrayOutputStream bos= new ByteArrayOutputStream(256);
		new ObjectOutputStream(bos).writeObject(wordCloud);
		p.add(Bytes.toBytes("CLOUD"), Bytes.toBytes("MW"),bos.toByteArray());
		table.put(p);	
		logger.debug("added wordCloud");
	}
	
	@SuppressWarnings("unchecked")
	public Set<String> getWordCloud(String word) throws IOException, ClassNotFoundException {
		System.out.println("fetching wordCloud for "+word);
		getConnection();
		Get g = new Get(Bytes.toBytes(word));
		Result r = wordCloudTable.get(g);
		byte[] value = r.getValue(Bytes.toBytes("CLOUD"), Bytes.toBytes("CLOUD"));
		if(value == null){
			return null;
		}
		Set<String> wordCloud = (Set<String>)new ObjectInputStream(new ByteArrayInputStream(value)).readObject();
		return wordCloud;
	}

}
