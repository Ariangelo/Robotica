package application;

import controller.FXMLControllerPID;
import javafx.application.Application;
import static javafx.application.Application.launch;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.ButtonType;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

public class MainApp extends Application {

    @Override
    public void start(Stage stage) throws Exception {
        FXMLLoader loader = new FXMLLoader(getClass().getResource("/fxml/ScenePID.fxml"));
        Parent root = loader.load();

        Scene scene = new Scene(root);
        scene.getStylesheets().add("/styles/Styles.css");

        stage.setTitle("Robótica.IO - Controle para robôs de auto equilibrio");
        stage.setScene(scene);
        stage.show();
        scene.getWindow().setOnCloseRequest((WindowEvent ev) -> {
            Alert alert = new Alert(Alert.AlertType.CONFIRMATION,
                    "Deseja realmente sair do sistema?",
                    ButtonType.YES, ButtonType.NO);
            alert.setTitle("Self-Balance");
            alert.showAndWait();
            if (alert.getResult() == ButtonType.NO) {
                ev.consume();
                alert.close();
            } else {
                FXMLControllerPID controller = loader.getController();
                controller.stopSerial();
            }
        });
    }

    /**
     * The main() method is ignored in correctly deployed JavaFX application.
     * main() serves only as fallback in case the application can not be
     * launched through deployment artifacts, e.g., in IDEs with limited FX
     * support. NetBeans ignores main().
     *
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        launch(args);
    }

}
