import java.util.ArrayList;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.FileReader;
import java.nio.file.FileAlreadyExistsException;

public class TextEdit {
    private final static int DEFAULT_SIZE = 25;

    private File f;
    private ArrayList<String> buff;
    private int[] cursor;

    public TextEdit(String fName) throws FileAlreadyExistsException {
        f = new File(fName);
        cursor = new int[2];
        buff = new ArrayList<>();

        if (f.isDirectory()) {
            throw new FileAlreadyExistsException("File is a directory");
        } else if (f.isFile()) {
            BufferedReader reader = new BufferedReader(new FileReader(f));
            String line;
            while (line = reader.readLine() != null) {
                buff.add(line);
            }
        }
    }

    public static void main(String[] args) throws IOException {
        if (args.length < 1) {
            System.out.println("Please specify a file name. Usage: `java TextEdit <filename>`");
            return;
        }
        String fName = args[0];
        TextEdit tEdit = new TextEdit(fName);
    }

    // move cursor left
    // move cursor right
    // move cursor up
    // move cursor down
    // insert char
    // insert newline
    // output buff
    // save
}