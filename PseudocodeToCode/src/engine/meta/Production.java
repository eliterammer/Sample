package engine.meta;

import java.util.ArrayList;
import java.util.List;

public class Production {
	public String name;
	public String value;
	public ArrayList<String> next;
	public List<String> components;
	public boolean isAutoGenerated = false;
	public boolean isToken=false;
}
