package controller;

import com.jfoenix.controls.JFXButton;
import com.jfoenix.controls.JFXComboBox;
import com.jfoenix.controls.JFXToggleButton;
import eu.hansolo.medusa.Gauge;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.ResourceBundle;
import javafx.application.Platform;
import javafx.beans.binding.Bindings;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.collections.FXCollections;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.PasswordField;
import javafx.scene.control.Slider;
import javafx.scene.control.Spinner;
import javafx.scene.control.SpinnerValueFactory;
import javafx.scene.control.TableView;
import javafx.scene.control.TextField;
import javafx.scene.layout.HBox;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import utility.Serial;
import utility.Ziegler;

public class FXMLControllerPID implements Initializable {

    @FXML
    private JFXComboBox cmbPortas;
    @FXML
    private JFXToggleButton btnConectar;
    @FXML
    private Label lblPorta;
    @FXML
    private HBox painelPID;
    @FXML
    private HBox painelGauge;
    @FXML
    private Slider sliderEsquerdo;
    @FXML
    private Slider sliderDireito;
    @FXML
    private TextField txtFldKp;
    @FXML
    private TextField txtFldKi;
    @FXML
    private TextField txtFldKd;
    @FXML
    private Gauge gaugeValor;
    @FXML
    private Spinner spinnerValor;
    @FXML
    private Spinner spinnerTempo;
    @FXML
    private TextField txtFldKu;
    @FXML
    private TextField txtFldTu;
    @FXML
    private Label lblDistancia;
    @FXML
    private TableView<Ziegler> tblView;
    @FXML
    private TextField txtFldServidor;
    @FXML
    private TextField txtFldPorta;
    @FXML
    private TextField txtFldUsuario;
    @FXML
    private PasswordField txtFldSenha;
    @FXML
    private Button btnConectarMQTT;
    @FXML
    private Button btnGravarEEPROM;

    private final Serial serial = new Serial();
    private final BooleanProperty conexao = new SimpleBooleanProperty(false);
    private final BooleanProperty conexaoMQTT = new SimpleBooleanProperty(false);
    private boolean primeiraVez = true;
    private double kP, kI, kD, kU = 0;
    private MqttClient client;
    private String topicoEntrada = "Sistemas.Embarcados.Topico.Entrada";  //tópico que sera assinado

    @FXML
    private void btnGravarEEPROMClick(ActionEvent e) throws MqttException {
        if (conexaoMQTT.get()) {
            String payload = "{'acao':'g'}";
            client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
        }
        if (conexao.get()) {
            serial.write("g\n");
        }
    }

    @FXML
    private void btnSetPointClick(ActionEvent e) {
        JFXButton btn = (JFXButton) e.getSource();
        SpinnerValueFactory.DoubleSpinnerValueFactory dblFactory = (SpinnerValueFactory.DoubleSpinnerValueFactory) spinnerValor.getValueFactory();
        dblFactory.setAmountToStepBy(Double.parseDouble(btn.getText()));
        spinnerValor.setValueFactory(dblFactory);
    }

    @FXML
    private void btnConectarClick(ActionEvent e) throws MqttException {
        if (conexaoMQTT.get()) {
            btnConectarMQTT.setText("Conectar");
            stopMQTT();
        } else {
            try {
                MqttConnectOptions connOpts = new MqttConnectOptions();
                connOpts.setCleanSession(true);
                connOpts.setAutomaticReconnect(true);
                connOpts.setConnectionTimeout(10);
                connOpts.setUserName(txtFldUsuario.getText());
                connOpts.setPassword(txtFldSenha.getText().toCharArray());
                client = new MqttClient(
                        "tcp://"
                        + txtFldServidor.getText() + ":"
                        + txtFldPorta.getText(),
                        MqttClient.generateClientId());
                client.setCallback(new MqttCallback() {
                    public void connectionLost(Throwable cause) {
                    }

                    public void messageArrived(String topic, MqttMessage message) throws Exception {
                        //System.out.println(message.toString());
                        mostraInfo(Long.toString(System.currentTimeMillis()) + ";" + message.toString());
                    }

                    public void deliveryComplete(IMqttDeliveryToken token) {
                    }
                });
                client.connect(connOpts);
                client.subscribe("Sistemas.Embarcados.Topico.Saida");
                conexaoMQTT.setValue(client.isConnected());
                btnConectarMQTT.setText("Desconectar");
                primeiraVez = true;
            } catch (MqttException ex) {
                conexaoMQTT.set(false);
                System.out.println(ex.getMessage());
            }
        }
    }

    @FXML
    private void btnClick(ActionEvent e) throws IOException, MqttException {
        JFXButton btn = (JFXButton) e.getSource();
        String payload;
        switch (btn.getId()) {
            case "btnHome":
                if (conexaoMQTT.get()) {
                    payload = "{'acao':'h'}";
                    client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
                }
                if (conexao.get()) {
                    serial.write("h\n");
                }
                break;
            case "btnEsquerda":
                break;
            case "btnDireita":
                break;
            case "btnCima":
                if (conexaoMQTT.get()) {
                    payload = "{'acao':'f'}";
                    client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
                }
                if (conexao.get()) {
                    serial.write("f\n");
                }
                break;
            case "btnBaixo":
                if (conexaoMQTT.get()) {
                    payload = "{'acao':'v'}";
                    client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
                }
                if (conexao.get()) {
                    serial.write("v\n");
                }
                break;
        }
    }

    @Override
    public void initialize(URL url, ResourceBundle rb) {
        cmbPortas.setItems(FXCollections.observableList(serial.detectPort()));
        cmbPortas.getSelectionModel().selectedItemProperty().addListener(
                (observable, oldValue, newValue) -> {
                    serial.setArdPort((String) newValue);
                });
        cmbPortas.getSelectionModel().selectFirst();
        cmbPortas.disableProperty().bind(conexao);

        btnConectar.selectedProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (newValue) {
                        if (startSerial()) {
                            btnConectar.setText("Desconectar");
                        } else {
                            btnConectar.setSelected(false);
                        }
                    } else {
                        stopSerial();
                        btnConectar.setText("Conectar");
                    }
                });

        sliderEsquerdo.valueProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (sliderEsquerdo.isFocused()) {
                        if (conexaoMQTT.get()) {
                            try {
                                String payload = "{'acao':'a', 'valor':" + newValue.floatValue() / 100 + "}";
                                client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
                            } catch (MqttException ex) {
                                System.out.println(ex.getMessage());
                            }
                        }
                        if (conexao.get()) {
                            serial.write("a" + newValue.floatValue() / 100 + "\n");
                        }
                    }
                });

        sliderDireito.valueProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (sliderDireito.isFocused()) {
                        if (conexaoMQTT.get()) {
                            try {
                                String payload = "{'acao':'b', 'valor':" + newValue.floatValue() / 100 + "}";
                                client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
                            } catch (MqttException ex) {
                                System.out.println(ex.getMessage());
                            }
                        }
                        if (conexao.get()) {
                            serial.write("b" + newValue.floatValue() / 100 + "\n");
                        }
                    }
                });

        spinnerValor.valueProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (spinnerValor.isFocused()) {
                        if (conexaoMQTT.get()) {
                            try {
                                String payload = "{'acao':'s', 'valor': " + newValue + "}";
                                client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
                            } catch (MqttException ex) {
                                System.out.println(ex.getMessage());
                            }
                        }
                        if (conexao.get()) {
                            serial.write("s" + newValue + "\n");
                        }
                    }
                });

        spinnerTempo.valueProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (spinnerTempo.isFocused()) {
                        if (conexaoMQTT.get()) {
                            try {
                                String payload = "{'acao':'t', 'valor': " + newValue + "}";
                                client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
                            } catch (MqttException ex) {
                                System.out.println(ex.getMessage());
                            }
                        }
                        if (conexao.get()) {
                            serial.write("t" + newValue + "\n");
                        }
                    }
                });
        /*
        tblView.getSelectionModel().selectedItemProperty().addListener(
                (observable, oldValue, newValue) -> {
                    System.out.println(newValue);
                    txtFldKp.setText(String.valueOf(newValue.getKp()));
                    txtFldKi.setText(String.valueOf(newValue.getKi()));
                    txtFldKd.setText(String.valueOf(newValue.getKd()));
                });

        txtFldKu.textProperty().addListener(
                (observable, oldValue, newValue) -> {
                    kU = Double.parseDouble(newValue);
                    processaZiegler(kU, Double.parseDouble(txtFldTu.getText()));
                });

        txtFldTu.textProperty().addListener(
                (observable, oldValue, newValue) -> {
                    processaZiegler(kU, Double.parseDouble(newValue));
                });
         */
        txtFldKp.textProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (!newValue.isEmpty()) {
                        kP = Double.parseDouble(newValue);
                        if (txtFldKp.isFocused()) {
                            if (conexaoMQTT.get()) {
                                try {
                                    String payload = "{'acao':'p', 'valor': " + newValue + "}";
                                    client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
                                } catch (MqttException ex) {
                                    System.out.println(ex.getMessage());
                                }
                            }
                            if (conexao.get()) {
                                serial.write("p" + newValue + "\n");
                            }
                        }
                    }
                });

        txtFldKi.textProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (!newValue.isEmpty()) {
                        kI = Double.parseDouble(newValue);
                        if (txtFldKi.isFocused()) {
                            if (conexaoMQTT.get()) {
                                try {
                                    String payload = "{'acao':'i', 'valor': " + newValue + "}";
                                    client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
                                } catch (MqttException ex) {
                                    System.out.println(ex.getMessage());
                                }
                            }
                            if (conexao.get()) {
                                serial.write("i" + newValue + "\n");
                            }
                        }
                    }
                });

        txtFldKd.textProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (!newValue.isEmpty()) {
                        kD = Double.parseDouble(newValue);
                        if (txtFldKd.isFocused()) {
                            if (conexaoMQTT.get()) {
                                try {
                                    String payload = "{'acao':'d', 'valor': " + newValue + "}";
                                    client.publish(topicoEntrada, new MqttMessage(payload.getBytes()));
                                } catch (MqttException ex) {
                                    System.out.println(ex.getMessage());
                                }
                            }
                            if (conexao.get()) {
                                serial.write("d" + newValue + "\n");
                            }
                        }
                    }
                });

        serial.getLine().addListener(
                (observable, oldValue, newValue) -> {
                    //System.out.println(newValue);
                    mostraInfo(newValue);
                });

        painelPID.disableProperty().bind(Bindings.and(conexao.not(), conexaoMQTT.not()));
        painelGauge.disableProperty().bind(painelPID.disableProperty());
        btnConectarMQTT.disableProperty().bind(txtFldServidor.textProperty().isEmpty().
                or(txtFldPorta.textProperty().isEmpty()));
        btnConectar.disableProperty().bind(conexaoMQTT);
        btnConectarMQTT.disableProperty().bind(conexao);

        txtFldServidor.setText("crobotica.deinfo.uepg.br");
        txtFldPorta.setText("1883");
        txtFldUsuario.setText("");
        txtFldSenha.setText("");

    }

    public void mostraInfo(String info) {
        String[] msg = info.split(";");
        if (msg.length == 11) {
            Platform.runLater(() -> {
                lblPorta.setText("Ângulo = " + msg[6]);
                if (primeiraVez) {
                    sliderEsquerdo.setValue(Double.parseDouble(msg[1]) * 100);
                    sliderDireito.setValue(Double.parseDouble(msg[2]) * 100);
                    kP = Double.parseDouble(msg[3]);
                    kU = 2 * kP;
                    kI = Double.parseDouble(msg[4]);
                    kD = Double.parseDouble(msg[5]);
                    txtFldKp.setText(msg[3]);
                    txtFldKi.setText(msg[4]);
                    txtFldKd.setText(msg[5]);
                    double setpoint = Double.parseDouble(msg[7]);
                    SpinnerValueFactory.DoubleSpinnerValueFactory dblFactory = (SpinnerValueFactory.DoubleSpinnerValueFactory) spinnerValor.getValueFactory();
                    dblFactory.setMin(setpoint - 45);
                    dblFactory.setMax(setpoint + 45);
                    spinnerValor.setValueFactory(dblFactory);
                    spinnerValor.getValueFactory().setValue(setpoint);
                    spinnerTempo.getValueFactory().setValue(Integer.parseInt(msg[8]));
                    //txtFldKu.setText(String.valueOf(kU));
                    primeiraVez = false;
                }
                gaugeValor.setValue(Double.parseDouble(msg[6]));
                lblDistancia.setText(msg[9]);
            });
        }
    }

    private boolean startSerial() {
        conexao.set(serial.connect());
        return conexao.get();
    }

    public void stopSerial() {
        if (conexao.get()) {
            serial.disconnect();
            conexao.set(false);
            System.out.println("Finalizando conexão");
        }
    }

    public void stopMQTT() throws MqttException {
        if (conexaoMQTT.get()) {
            client.disconnect();
            conexaoMQTT.set(false);
            System.out.println("Finalizando conexão MQTT");
        }
    }

    private void processaZiegler(double valKu, double tU) {
        List list = new ArrayList();
        kU = valKu;
        kP = 0.5 * kU;
        list.add(new Ziegler("P", kP));
        kP = 0.45 * kU;
        kI = 1.2 * kP / tU;
        list.add(new Ziegler("PI", kP, kI));
        kP = 0.6 * kU;
        kI = 2 * kP / tU;
        kD = kP * tU / 8;
        list.add(new Ziegler("PID", kP, kI, kD));
        tblView.setItems(FXCollections.observableList(list));
    }
}
