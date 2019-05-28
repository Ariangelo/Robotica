package controller;

import com.jfoenix.controls.JFXButton;
import com.jfoenix.controls.JFXComboBox;
import com.jfoenix.controls.JFXToggleButton;
import eu.hansolo.medusa.Gauge;
import java.io.IOException;
import java.net.URL;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.ResourceBundle;
import javafx.application.Platform;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.collections.FXCollections;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.chart.LineChart;
import javafx.scene.control.Label;
import javafx.scene.control.Slider;
import javafx.scene.control.Spinner;
import javafx.scene.control.TableView;
import javafx.scene.control.TextField;
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

    private final Serial serial = new Serial();
    private final BooleanProperty conexao = new SimpleBooleanProperty(false);
    private final IntegerProperty deslocamentoH = new SimpleIntegerProperty(0);
    private final IntegerProperty velocidade = new SimpleIntegerProperty(0);
    private boolean primeiraVez = true;
    private double kP, kI, kD, kU = 0;

    @FXML
    private void btnClick(ActionEvent e) throws IOException {
        String s;
        JFXButton btn = (JFXButton) e.getSource();
        switch (btn.getId()) {
            case "btnHome":
                deslocamentoH.set(0);
                serial.write("s97,5" + "\n");
                break;
            case "btnEsquerda":
                break;
            case "btnDireita":
                break;
            case "btnCima":
                deslocamentoH.set(0);
                serial.write("s98.5" + "\n");
                break;
            case "btnBaixo":
                deslocamentoH.set(0);
                serial.write("s96.5" + "\n");
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
                    float valor = newValue.floatValue() / 100;
                    serial.write("a" + valor + "\n");
                });

        sliderDireito.valueProperty().addListener(
                (observable, oldValue, newValue) -> {
                    float valor = newValue.floatValue() / 100;
                    serial.write("b" + valor + "\n");
                });

        spinnerValor.valueProperty().addListener(
                (observable, oldValue, newValue) -> {
                    serial.write("s" + newValue + "\n");
                });

        spinnerTempo.valueProperty().addListener(
                (observable, oldValue, newValue) -> {
                    serial.write("t" + newValue + "\n");
                });

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

        txtFldKp.textProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (!newValue.isEmpty()) {
                        kP = Double.parseDouble(newValue);
                        serial.write("p" + newValue + "\n");
                    }
                });

        txtFldKi.textProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (!newValue.isEmpty()) {
                        kI = Double.parseDouble(newValue);
                        serial.write("i" + newValue + "\n");
                    }
                });

        txtFldKd.textProperty().addListener(
                (observable, oldValue, newValue) -> {
                    if (!newValue.isEmpty()) {
                        kD = Double.parseDouble(newValue);
                        serial.write("d" + newValue + "\n");
                    }
                });

        serial.getLine().addListener(
                (observable, oldValue, newValue) -> {
                    //System.out.println(newValue);
                    String[] msg = newValue.split(";");
                    if (msg.length == 9) {
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
                                txtFldKu.setText(String.valueOf(kU));
                                primeiraVez = false;
                            }
                            gaugeValor.setValue(Double.parseDouble(msg[6]));
                            DecimalFormat df = new DecimalFormat("0.#");
                            lblDistancia.setText(df.format(Double.parseDouble(msg[8])));
                        });
                    }
                });

        //btnCalibrar.disableProperty().bind(conexao.not());
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
