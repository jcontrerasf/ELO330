// Very simple client that just sends
// lines to the server and reads lines
// that the server sends.
// Compilar con $ javac Visualizador.java
// Ejecutar con $ java Visualizador [server name] [port]
import java.net.*;
import java.io.*;

public class Visualizador {
  public static void main(String[] args) throws IOException {
    // Passing null to getByName() produces the
    // special "Local Loopback" IP address, for
    // testing on one machine w/o a network:
    InetAddress addr =  InetAddress.getByName(args[0]);
    // Alternatively, you can use 
    // the address or name:
    // InetAddress addr = InetAddress.getByName("127.0.0.1");
    // InetAddress addr = InetAddress.getByName("localhost");
    System.out.println("addr = " + addr);
    Socket socket = new Socket(addr, Integer.parseInt(args[1]));
    // Guard everything in a try-finally to make
    // sure that the socket is closed:
    try {
      System.out.println("socket = " + socket);
      BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
      // Output is automatically flushed
      // by PrintWriter:
      //PrintWriter out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(
      //                                                      socket.getOutputStream())),true);
      for(;;) {
        //out.println("How are you all doing " + i);
        String str = in.readLine();
        System.out.println(str);
      }
      //out.println("BYE");
    } finally {
      System.out.println("closing...");
      socket.close();
    }
  }
}
