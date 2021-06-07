
#include <GL/glut.h>
#include <stdio.h>


#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))

#define TEXTURE_FILE "texture.bmp"

GLfloat light_diffuse[] = {0.0, 1.0, 1.0, 0.0};
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};
GLfloat n[6][3] = { // Normals for the 6 faces of a cube
    {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
    {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}};
GLint faces[6][4] = { // Vertex indices for the 6 faces of a cube
    {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4}, {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3}};
GLfloat v[8][3]; // Will be filled in with X,Y,Z vertexes

double pos = 0;


void drawBox(void) {
    int i;

    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    glRotatef(pos, 0.0, 0.0, 1.0);

    for (i = 0; i < 6; i++) {
        glBegin(GL_POLYGON);
        glNormal3fv(&n[i][0]);
        glTexCoord2f(1.0, 0.0);
        glVertex3fv(&v[faces[i][0]][0]);
        glTexCoord2f(1.0, 1.0);
        glVertex3fv(&v[faces[i][1]][0]);
        glTexCoord2f(0.0, 1.0);
        glVertex3fv(&v[faces[i][2]][0]);
        glTexCoord2f(0.0, 0.0);
        glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
    glPopMatrix();
    glFlush();
    glEnable(GL_TEXTURE_2D);

    pos += 0.5;
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBox();
    glutSwapBuffers();
}

void timer(int t) {
    display();
    glutTimerFunc(t, timer, 0);
}

void keyboard(unsigned char key, int UNUSED(x), int UNUSED(y)) {
    // Exit key ESC
    if (key == 27) exit(0);
}

GLuint load_texture(const char *filename) {
    GLuint texture;
    int width, height;
    unsigned char *data;

    FILE *file;
    file = fopen(filename, "rb");

    if (file == NULL)
        return 0;
    width = 1000;
    height = 1000;
    data = (unsigned char *)malloc(width * height * 3);
    fread(data, width * height * 3, 1, file);
    fclose(file);

    for (int i = 0; i < width * height; ++i) {
        int index = i * 3;
        unsigned char B, R;
        B = data[index];
        R = data[index + 2];

        data[index] = R;
        data[index + 2] = B;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    free(data);

    return texture;
}

void init(void) {
    // Setup cube vertex data
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -1;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = 1;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -1;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = 1;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = 1;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = -1;

    // Enable a single OpenGL light
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    // Use depth buffering for hidden surface elimination
    glEnable(GL_DEPTH_TEST);

    // Setup the view of the cube
    glMatrixMode(GL_PROJECTION);
    gluPerspective(/* field of view in degree */ 40.0,
                   /* aspect ratio */ 1.0,
                   /* Z near */ 1.0, /* Z far */ 10.0);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
              0.0, 0.0, 0.0,  /* center is at (0,0,0) */
              0.0, 1.0, 0.0); /* up is in positive Y direction */

    GLuint texture = load_texture("texture.bmp");
    glBindTexture(GL_TEXTURE_2D, texture);

    // Adjust cube position to be asthetic angle.
    glTranslatef(0.0, 0.0, -1.0);
    glRotatef(60, 1.0, 0.0, 0.0);
    glRotatef(-20, 0.0, 0.0, 1.0);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("red 3D lighted cube");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    init();
    timer(10);
    glutMainLoop();

    return 0;
}
