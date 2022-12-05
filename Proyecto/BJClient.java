import javafx.application.Application;
import javafx.stage.Stage;
import javafx.scene.Scene;
//import javafx.scene.layout.StackPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.BorderPane;
import javafx.scene.control.Button;
import javafx.scene.control.TextField;
import java.net.*;
import java.io.*;

public class BJClient extends Application {

    @Override // Override the start method in the Application class
    public void start(Stage primaryStage) {
        BorderPane pane = new BorderPane();

        TextField ip = new TextField("aragorn.elo.utfsm.cl");
        //ip.setPromptText("Ej: aragorn.elo.utfsm.cl");
        TextField port = new TextField("47200");
        Button btnConectar = new Button("Conectar");
        btnConectar.setOnAction(e -> {
            try{
                conectar(ip.getText(), port.getText());
            } catch (IOException ex){
                System.out.println("Error al conectar");
            }
        });
        HBox hbox = new HBox(8); //spacing = 8
        hbox.getChildren().addAll(ip, port, btnConectar);
        pane.setTop(hbox);

        Button newCard = new Button("Sacar otra carta");
        pane.setCenter(newCard);
        Scene scene = new Scene(pane, 500, 200);
        primaryStage.setTitle("Simple Black Jack"); // Set the stage title
        primaryStage.setScene(scene); // Place the scene in the stage
        primaryStage.show(); // Display the stage
    }

    //Usar Threads aquí, sino se queda pegado esperando en el for
    private void conectar(String ip, String port) throws IOException {
        InetAddress addr =  InetAddress.getByName(ip);
        System.out.println("addr = " + addr);
        Socket socket = new Socket(addr, Integer.parseInt(port));
        try {
            System.out.println("socket = " + socket);
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(
                                                                  socket.getOutputStream())),true);
            for (; ; ) {
                out.println("a");
                out.println("a"); //para probar con el servidor que tenemos
                String str = in.readLine();
                if (str != null) {
                    System.out.println(str);
                }
            }
            //out.println("BYE");
        } finally {
            System.out.println("closing...");
            socket.close();
        }
    }

    public static void main(String[] args) {
        launch(args);
    }
}
