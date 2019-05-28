package utility;

import com.fazecast.jSerialComm.SerialPort;
import com.fazecast.jSerialComm.SerialPortDataListener;
import com.fazecast.jSerialComm.SerialPortEvent;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

/*
import jssc.SerialPort;
import jssc.SerialPortException;
import jssc.SerialPortList;
 */
public class Serial {

    private String ardPort;
    private SerialPort serPort;

    public static final String SEPARATOR = ";";
    private StringBuilder sb = new StringBuilder();
    private final StringProperty line = new SimpleStringProperty("");

    public Serial() {
        ardPort = "";
    }

    public Serial(String port) {
        ardPort = port;
    }

    /* connect() looks for a valid serial port with an Arduino board connected.
    * If it is found, it's opened and a listener is added, so everytime 
    * a line is returned the stringProperty is set with that line. 
    * For that, a StringBuilder is used to store the chars and extract the line 
    * content whenever a '\r\n' is found.
     */
    public boolean connect(String ardPort) {
        this.ardPort = ardPort;
        return connect();
    }

    public boolean connect() {
        StringBuilder message = new StringBuilder();
        Arrays.asList(SerialPort.getCommPorts())
                .stream()
                .filter(name -> ((!ardPort.isEmpty() && name.getSystemPortName().equals(ardPort)) || (ardPort.isEmpty())))
                .findFirst()
                .ifPresent(name -> {
                    serPort = name;
                    System.out.println("Connecting to " + serPort.getSystemPortName());
                    if (serPort.openPort()) {
                        System.out.println("Conected to " + serPort.getSystemPortName());
                        serPort.setBaudRate(115200);
                        serPort.addDataListener(new SerialPortDataListener() {
                            @Override
                            public int getListeningEvents() {
                                return SerialPort.LISTENING_EVENT_DATA_RECEIVED;
                            }

                            @Override
                            public void serialEvent(SerialPortEvent event) {
                                byte[] newData = event.getReceivedData();
                                for (int i = 0; i < newData.length; ++i) {
                                    sb.append((char) newData[i]);
                                    String ch = sb.toString();
                                    if (ch.endsWith("\r\n")) {
                                        // add timestamp
                                        line.set(Long.toString(System.currentTimeMillis())
                                                .concat(SEPARATOR)
                                                .concat(ch.substring(0, ch.indexOf("\r\n"))));
                                        sb = new StringBuilder();
                                    }
                                }
                            }
                        });
                    } else {
                        System.out.println("ERRO: Port " + serPort.getSystemPortName());
                    }
                });
        return serPort != null;
    }

    public void write(int[] text) {
        /*  try {
            serPort.writeIntArray(text);
        } catch (SerialPortException ex) {
            System.out.println("ERROR: writing '" + text + "': " + ex.toString());
        }*/

    }

    public void write(String text) {
        try {
            System.out.println("Writing: " + text);
            serPort.getOutputStream().write(text.getBytes());
            serPort.getOutputStream().flush();
        } catch (IOException ex) {
            System.out.println("ERROR: writing '" + text + "': " + ex.toString());
        }
    }

    public void disconnect() {
        if (serPort != null) {
            serPort.removeDataListener();
            if (serPort.isOpen()) {
                serPort.closePort();
            }
            System.out.println("Disconnecting: comm port closed.");
        }
    }

    public StringProperty getLine() {
        return line;
    }

    public void setArdPort(String ardPort) {
        this.ardPort = ardPort;
    }

    public String getArdPort() {
        return ardPort;
    }

    public String getPortName() {
        return serPort != null ? serPort.getSystemPortName() : "";
    }

    public List<String> detectPort() {
        List<String> ports = new ArrayList<>();
        for (SerialPort p : SerialPort.getCommPorts()) {
            ports.add(p.getSystemPortName());
        }
        return ports.stream().sorted().collect(Collectors.toList());
    }
}
