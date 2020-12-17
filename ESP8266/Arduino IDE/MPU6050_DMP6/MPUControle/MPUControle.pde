import processing.serial.*;
import processing.opengl.*;
import toxi.geom.*;
import toxi.processing.*;

ToxiclibsSupport gfx;

Serial porta;                         
char[] infoSensor = new char[14]; 
int contadorSerial = 0; 
int sincronizado = 0;
int intervalo = 0;

float[] q = new float[4];
Quaternion quat = new Quaternion(1, 0, 0, 0);

void setup() {
    size(300, 300, OPENGL);
    gfx = new ToxiclibsSupport(this);
    lights();
    smooth();
    String nomePorta = Serial.list()[0];
    porta = new Serial(this, nomePorta, 115200);
    porta.write('r');
}

void draw() {   
  if (millis() - intervalo > 1000) {
        porta.write('r');
        intervalo = millis();
    }    
    background(0);
    pushMatrix();
    translate(width / 2, height / 2);
    float[] axis = quat.toAxisAngle();
    rotate(axis[0], -axis[1], axis[3], axis[2]);
    fill(255, 0, 0, 200);
    box(10, 10, 200);
    fill(0, 0, 255, 200);
    pushMatrix();
    translate(0, 0, -120);
    rotateX(PI / 2);
    desenharCilindro(0, 10, 20, 16);
    popMatrix();
    fill(0, 255, 0, 200);
    beginShape(TRIANGLES);
    vertex(-100,  2, 30); vertex( 0,   2, -80); vertex(100,  2, 30);
    vertex(-100, -2, 30); vertex( 0,  -2, -80); vertex(100, -2, 30);
    vertex(  -2,  0, 98); vertex(-2, -30,  98); vertex( -2,  0, 70);
    vertex(   2,  0, 98); vertex( 2, -30,  98); vertex(  2,  0, 70); 
    endShape();
    beginShape(QUADS);
    vertex(-100,   2, 30); vertex(-100,  -2, 30); vertex(  0,  -2, -80); vertex(  0,   2, -80);
    vertex( 100,   2, 30); vertex( 100,  -2, 30); vertex(  0,  -2, -80); vertex(  0,   2, -80);
    vertex(-100,   2, 30); vertex(-100,  -2, 30); vertex(100,  -2,  30); vertex(100,   2,  30);
    vertex(  -2,   0, 98); vertex(   2,   0, 98); vertex(  2, -30,  98); vertex( -2, -30,  98);
    vertex(  -2,   0, 98); vertex(   2,   0, 98); vertex(  2,   0,  70); vertex( -2,   0,  70);
    vertex(  -2, -30, 98); vertex(   2, -30, 98); vertex(  2,   0,  70); vertex( -2,   0,  70);
    endShape();  
    popMatrix();
}

void serialEvent(Serial porta) {
    intervalo = millis();
    while (porta.available() > 0) {
        int c = porta.read();
        if (sincronizado == 0 && c != '$') return; 
        sincronizado = 1;
        if ((contadorSerial == 1 && c != 2)
            || (contadorSerial == 12 && c != '\r')
            || (contadorSerial == 13 && c != '\n'))  {
            contadorSerial = 0;
            sincronizado = 0;
            return;
        }
        if (contadorSerial > 0 || c == '$') {
            infoSensor[contadorSerial++] = (char)c;
            if (contadorSerial == 14) {
                contadorSerial = 0;            
                q[0] = ((infoSensor[2] << 8) | infoSensor[3]) / 16384.0f;
                q[1] = ((infoSensor[4] << 8) | infoSensor[5]) / 16384.0f;
                q[2] = ((infoSensor[6] << 8) | infoSensor[7]) / 16384.0f;
                q[3] = ((infoSensor[8] << 8) | infoSensor[9]) / 16384.0f;
                for (int i = 0; i < 4; i++) if (q[i] >= 2) q[i] = -4 + q[i];                
                quat.set(q[0], q[1], q[2], q[3]);
                println("q:\t" + 
                  round(q[0] * 100.0f) / 100.0f + "\t" + 
                  round(q[1] * 100.0f) / 100.0f + "\t" + 
                  round(q[2] * 100.0f) / 100.0f + "\t" + 
                  round(q[3] * 100.0f) / 100.0f); 
            }
        }
    }
}

void desenharCilindro(float raioTopo, float raioInferior, float altura, int lados) {
    float angulo = 0;
    float incrementoAngulo = TWO_PI / lados;
    beginShape(QUAD_STRIP);
    for (int i = 0; i < lados + 1; ++i) {
        vertex(raioTopo * cos(angulo), 0, raioTopo * sin(angulo));
        vertex(raioInferior * cos(angulo), altura, raioInferior * sin(angulo));
        angulo += incrementoAngulo;
    }
    endShape();
    if (raioTopo != 0) {
        angulo = 0;
        beginShape(TRIANGLE_FAN);
        vertex(0, 0, 0);
        for (int i = 0; i < lados + 1; i++) {
            vertex(raioTopo * cos(angulo), 0, raioTopo * sin(angulo));
            angulo += incrementoAngulo;
        }
        endShape();
    }
    if (raioInferior != 0) {
        angulo = 0;
        beginShape(TRIANGLE_FAN);
        vertex(0, altura, 0);
        for (int i = 0; i < lados + 1; i++) {
            vertex(raioInferior * cos(angulo), altura, raioInferior * sin(angulo));
            angulo += incrementoAngulo;
        }
        endShape();
    }       
}
