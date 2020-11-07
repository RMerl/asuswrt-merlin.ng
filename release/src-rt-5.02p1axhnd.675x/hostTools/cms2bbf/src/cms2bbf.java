import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.BufferedWriter;
import java.io.FileWriter;

//import java.util.ArrayList;
//import java.util.List;

import org.xml.sax.InputSource;

import com.broadcom.bcacpe.cmsxml2bbfxml.Cms2BbfParserHandler;

public class cms2bbf {
    private static void parseDataModels(String inFile, String outFile)
    {
        try {
            File file = new File(inFile);
            InputStream inputStream = new FileInputStream(file);
            Reader reader = new InputStreamReader(inputStream, "UTF-8");
            InputSource inputSource = new InputSource(reader);
            inputSource.setEncoding("UTF-8");
            BufferedWriter writer = new BufferedWriter(new FileWriter(outFile));
            // use the following code to APPEND to existed file
            //BufferedWriter writer = new BufferedWriter(new FileWriter(outFile, true));

            new Cms2BbfParserHandler(inputSource, writer);
            
            writer.close(); writer = null;
            inputSource = null;
            reader.close(); reader = null;
            inputStream.close(); inputStream = null;
            file = null;
        } catch (IOException e) {
            System.out.println("IO error");
            System.out.println("Exception: " + e.getMessage() + " " + e.getClass().getName());
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
    	if (args.length < 2) {
            System.out.println("USAGE: java <program name> <input file> <output file>");
            return;
    	}

        parseDataModels(args[0], args[1]);
    }
}
