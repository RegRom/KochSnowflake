#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <cmath>
#include <time.h>
#include <iostream>
#include <vector>
#include <array>

#define _USE_MATH_DEFINES
const double PI = 3.141592653589793238463;

//Wspó³rzêdne, od których zaczynamy rysowanie
GLfloat beginX = -35, beginY = -55;
//Zmienna przechowujaca wylosowany kolor krawêdzi p³atka
GLfloat edgeColor[3];
//Zmienna do przechowywania koloru sprawdzanych s¹siadów kolorowanego piksela
GLfloat curColor[3];
//Wektor wektorów, przechowuj¹cy narysowane krawêdzie
std::vector <std::array<GLfloat, 4>> edges;
GLint edgeNum;

//Funkcja losuj¹ca liczbê zmiennoprzecinkow¹ do okreœlania koloru
float myRand()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

//Funkcja maluj¹ca piksel w okreœlonym rozmiarze
void draw_pixel(GLint x, GLint y)
{
	glColor3f(0.0, 1.0, 1.0);
	glPointSize(0.1);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}

//Funkcja pomocnicza do funkcji scanFill wykrywaj¹ca krawêdzie p³atka
void edgeDetect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLint *le, GLint *re)
{
	GLfloat temp, x, mx;
	GLint i;

	//Jeœli wspó³rzêdna y punktu koñcowego jest wiêksza od wspó³rzêdnej y punktu pocz¹tkowego to zamieniamy
	if (y1 > y2)
	{
		temp = x1, x1 = x2, x2 = temp;
		temp = y1, y1 = y2, y2 = temp;
	}

	//jeœli prosta jest pozioma to przyrost x jest ró¿nic¹ wartoœci na osi x
	if (y1 == y2)
		mx = x2 - x1;
	//jeœli prosta nie jest pozioma to obliczamy przyrost x ze wzoru
	else
		mx = (x2 - x1) / (y2 - y1);

	x = x1;

	//Wyznaczamy wspó³rzêdne na osi x, by okreœliæ pocz¹tek i koniec malowanej linii pikseli
	for (i = int(y1); i <= (int)y2; i++)
	{
		if (x < (float)le[i]) le[i] = (int)x;
		if (x > (float)re[i]) re[i] = (int)x;
		x += mx;
	}
}

//Funkcja wykonuj¹ca algorytm Scan Fill, maluj¹ca piksele - linia po linii
void scanFill()
{
	edgeNum = edges.size();
	GLint *le = new GLint[edgeNum];
	GLint *re = new GLint[edgeNum]; 
	GLint i, j;

	//zape³nienie tablic wartoœciami skrajnymi, tak by piksele na pewno mieœci³y siê w przedziale
	for (i = 0; i < edgeNum; i++)
		le[i] = 5000, re[i] = 0;

	//Wykrywanie wszystkich krawêdzi p³atka
	for (unsigned int i = 0; i < edges.size(); i++)
	{
		edgeDetect(edges[i][0], edges[i][1], edges[i][2], edges[i][3], le, re);
	}

	//Malowanie pikseli
	for (j = 0; j < 500; j++)
	{
		if (le[j] <= re[j])
			for (i = le[j]; i < re[j]; i++)
				draw_pixel(i, j);
	}
}

//Funkcja pomocnicza do funkcji floodFill, porównuj¹ca dwa kolory
int checkColor(float a[], float b[])
{
	if ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]))
		return 0;
	return 1;
}

//Funkcja realizuj¹ca algorytm Flood Fill, sprawdzaj¹ca s¹siednie piksele i maluj¹ca jeœli ich kolor jest inny niz krawêdzi
void floodFill(GLfloat x, GLfloat y)
{
	//Rekurencyjne wywo³ujemy funkcjê dla czterech s¹siednich pikseli:
	//-wy¿ej
	//-ni¿ej
	//-po lewo
	//-po prawo

	//Funkcj¹ glReadPixels() odczytujemy informacje o pikselu, a dok³adniej jego kolor
	glReadPixels(x - 1, y, 1, 1, GL_RGB, GL_FLOAT, curColor);
	if (checkColor(edgeColor, curColor))
	{
		floodFill(x - 1, y);
	}

	glReadPixels(x, y - 1, 1, 1, GL_RGB, GL_FLOAT, curColor);
	if (checkColor(edgeColor, curColor))
	{
		floodFill(x, y - 1);
	}

	glReadPixels(x + 1, y, 1, 1, GL_RGB, GL_FLOAT, curColor);
	if (checkColor(edgeColor, curColor))
	{
		floodFill(x + 1, y);
	}

	glReadPixels(x, y + 1, 1, 1, GL_RGB, GL_FLOAT, curColor);
	if (checkColor(edgeColor, curColor))
	{
		floodFill(x, y + 1);
	}
}

/******************************************************/

//Funkcja rysuj¹ca p³atek Kocha
void drawKoch(GLfloat dir, GLfloat len, GLint iter) 
{
	//Zmienna pomocnicza do przeliczenia stopni na radiany
	GLdouble dirRad = dir * PI / 180;
	GLfloat newX = beginX + len * cos(dirRad);
	GLfloat newY = beginY + len * sin(dirRad);

	//Rysujemy krzyw¹ Kocha, liczba iteracji okreœla g³êbokoœæ krzywej Kocha
	//Przypadek podstawowy to rysowanie prostej linii
	if (iter == 0) 
	{
		glVertex2f(beginX, beginY);
		glVertex2f(newX, newY);

		beginX = newX;
		beginY = newY;
		std::array<GLfloat, 4> tmpTab = { beginX, beginY, newX, newY };
		edges.push_back(tmpTab);
	}

	//Zgodnie z okreœlon¹ g³êbokoœci¹ fraktala, wywo³ujemy rekurencyjnie funkcjê rysuj¹c¹ i dekrementujemy iterator
	else 
	{
		iter--;
		//Rysujemy cztery czêœci ³amanej _/\_ by to zrobiæ, wywo³ujemy rekurencyjnie funkcjê,
		//obracaj¹c rysowany fragment prostej o zadan¹ liczbê stopni
		drawKoch(dir, len, iter);
		dir += 60.0;
		drawKoch(dir, len, iter);
		dir -= 120.0;
		drawKoch(dir, len, iter);
		dir += 60.0;
		drawKoch(dir, len, iter);
	}
}

//Funkcja przerysowuj¹ca scenê
void RenderScene() 
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBegin(GL_LINES);
	glColor3f(edgeColor[0], edgeColor[1], edgeColor[2]);

	//W zale¿noœci od po¿¹danej g³êbokoœci fraktala, wywo³ujemy funkcje n razy
	//K¹t pocz¹tkowy jaki podajemy do funkcji ma wartoœæ 360/n * k
	//gdzie k to liczba ca³kowita z przedzia³u <0,n)
	drawKoch(0.0, 0.1, 6);
	drawKoch(60.0, 0.1, 6);
	drawKoch(120.0, 0.1, 6);
	drawKoch(180.0, 0.1, 6);
	drawKoch(240.0, 0.1, 6);
	drawKoch(300.0, 0.1, 6);

	glEnd();

	scanFill();
	//floodFill(0.0f, 0.0f);

	glFlush();
}

void ChangeSize(GLsizei horizontal, GLsizei vertical)
// Parametry horizontal i vertical (szerokoœæ i wysokoœæ okna) s¹
// przekazywane do funkcji za ka¿dym razem, gdy zmieni siê rozmiar okna
{
	GLfloat AspectRatio;
	// Deklaracja zmiennej AspectRatio okreœlaj¹cej proporcjê wymiarów okna

	if (vertical == 0)
		// Zabezpieczenie pzred dzieleniem przez 0
		vertical = 1;

	glViewport(0, 0, horizontal, vertical);
	// Ustawienie wielkoœciokna okna urz¹dzenia (Viewport)
	// W tym przypadku od (0,0) do (horizontal, vertical)

	glMatrixMode(GL_PROJECTION);
	// Okreœlenie uk³adu wspó³rzêdnych obserwatora

	glLoadIdentity();
	// Okreœlenie przestrzeni ograniczaj¹cej

	AspectRatio = (GLfloat)horizontal / (GLfloat)vertical;
	// Wyznaczenie wspó³czynnika proporcji okna

	// Gdy okno na ekranie nie jest kwadratem wymagane jest
	// okreœlenie okna obserwatora.
	// Pozwala to zachowaæ w³aœciwe proporcje rysowanego obiektu
	// Do okreœlenia okna obserwatora s³u¿y funkcja glOrtho(...)

	if (horizontal <= vertical)
		glOrtho(-100.0, 100.0, -100.0 / AspectRatio, 100.0 / AspectRatio, 1.0, -1.0);

	else
		glOrtho(-100.0*AspectRatio, 100.0*AspectRatio, -100.0, 100.0, 1.0, -1.0);

	glMatrixMode(GL_MODELVIEW);
	// Okreœlenie uk³adu wspó³rzêdnych     

	glLoadIdentity();
}

void MyInit(void)
{
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	// Kolor okna wnêtrza okna - ustawiono na szary
}

int main(int argc, char** argv)
{
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

	for (size_t i = 0; i < 3; i++)
	{
		edgeColor[i] = myRand();
		curColor[i] = 0.0f;
	}

	glutCreateWindow("Koch Snowflake");
	glutDisplayFunc(RenderScene);

	glutReshapeFunc(ChangeSize);
	MyInit();

	glutMainLoop();
}