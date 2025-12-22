import java.util.ArrayList;
import java.lang.StringBuilder;
import java.io.*;
import javax.swing.*;
import java.awt.event.*;

public class TextEdit {
    private File f;
    private ArrayList<char[]> buff;
    private int[] cursor;
    private int topLine;
    private boolean insertMode;

    public TextEdit(String fName) throws IOException {
        f = new File(fName);
        cursor = new int[2];
        topLine = 0;
        buff = new ArrayList<>();
        insertMode = false;

        if (f.isDirectory()) {
            System.out.println("File is a directory");
        } else if (f.isFile()) {
            BufferedReader reader = new BufferedReader(new FileReader(f));
            String line;
            while ((line = reader.readLine()) != null) {
                buff.add(line.toCharArray());
            }
            reader.close();
        }
    }

    public static void main(String[] args) throws IOException {
        if (args.length < 1) {
            System.out.println("Please specify a file name. Usage: `java TextEdit <filename>`");
            return;
        }
        String fName = args[0];
        TextEdit tEdit = new TextEdit(fName);
        JFrame frame = new JFrame();
        boolean cont = true;
        int next;
        initFrame(frame, tEdit);

        while (cont) {
            //tEdit.output();
            //next = in.read();
            //cont = handleUserInput(next, inputMode, tEdit);
        }
        // in.close();
        tEdit.save();
    }

    public void output() {
        System.out.println(CLEAR_STRING);
        int numLines = Math.min(DEFAULT_SIZE, buff.size() - topLine);
        StringBuilder outBuilder = new StringBuilder();
        for (int i = 0; i < numLines; i++) {
            char[] line = buff.get(topLine + i);
            if (topLine + i == cursor[0]) {
                line[cursor[1]] = '\u2588';
            }
            outBuilder.append(line);
        }
        System.out.println(outBuilder);
    }

    public boolean save() throws IOException {
        if (f.isDirectory()) {
            return false;
        }
        if (!f.isFile()) {
            f.createNewFile();
        }

        if (f.isFile()) {
            BufferedWriter writer = new BufferedWriter(new FileWriter(f));
            for (char[] l : buff) {
                writer.write(String.valueOf(l));
                writer.newLine();
            }
            writer.close();
            return true;
        }
        return false;
    }

    public void exitInsertMode() {
        System.out.println("exiting insert mode");
        insertMode = false;
    }

    public void enterInsertMode() {
        insertMode = true;
    }

    private static void initFrame(JFrame frame, TextEdit tEdit) {
        frame.setSize(800, 600);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        JComponent content = (JComponent) frame.getContentPane();

        content.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW)
                .put(KeyStroke.getKeyStroke("ESCAPE"), "exitInsert");
        content.getActionMap().put("exitInsert", new AbstractAction() {
            @Override
            public void actionPerformed(ActionEvent e) {
                tEdit.exitInsertMode();
            }
        });

        frame.setVisible(true);
    }

    // move cursor left
    // move cursor right
    // move cursor up
    // move cursor down
    // insert char
    // insert newline
    // output buff
    // save

    private static final String CLEAR_STRING = "\033[H\033[2J";
    private final static int DEFAULT_SIZE = 25;
}
