/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package utility;

/**
 *
 * @author ari
 */
public class Ziegler {
    private String tipoControle = "";
    private double kP = 1;
    private double kI = 0;
    private double kD = 0;

    public Ziegler() {
    }

    public Ziegler(String tipoControle, double kP) {
        this.tipoControle = tipoControle;
        this.kP = kP;
    }

    public Ziegler(String tipoControle, double kP, double kI) {
        this.tipoControle = tipoControle;
        this.kP = kP;
        this.kI = kI;
    }

    public Ziegler(String tipoControle, double kP, double kI, double kD) {
        this.tipoControle = tipoControle;
        this.kP = kP;
        this.kI = kI;
        this.kD = kD;
    }

    public String getTipoControle() {
        return tipoControle;
    }

    public void setTipoControle(String tipoControle) {
        this.tipoControle = tipoControle;
    }

    public double getKp() {
        return kP;
    }

    public void setKp(double kP) {
        this.kP = kP;
    }

    public double getKi() {
        return kI;
    }

    public void setKi(double kI) {
        this.kI = kI;
    }

    public double getKd() {
        return kD;
    }

    public void setKd(double kD) {
        this.kD = kD;
    }

}
