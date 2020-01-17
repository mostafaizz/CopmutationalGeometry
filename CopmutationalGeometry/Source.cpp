#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <opencv2\opencv.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\highgui.hpp>


using namespace std;

#define X 0
#define Y 1

typedef vector<int> PointInt;

struct Vertex
{
	int vNum;				// vertex number
	PointInt v;				// coordinates
	bool ear;				// true if an ear
	Vertex *next, *prev;	// circular list
};

class Polygon
{
private:
	int nVertices;
	Vertex* head;
public:
	Polygon() {
		head = 0; 
		nVertices = 0;
	}

	// inserting new vertex from Vertex
	bool insertVertex(Vertex *v)
	{
		if (head == 0)
		{
			head = v;
			head->prev = head->next = head;
		}
		else
		{
			Vertex* tmp = head->prev;
			tmp->next = v;
			v->next = head;
			head->prev = v;
			v->prev = tmp;
		}
		v->vNum = nVertices++;
		return true;
	}

	// inserting new vertex from pointInt
	bool insertVertex(PointInt pt)
	{
		Vertex * v = new Vertex;
		v->v = pt;
		if (v == 0)
		{// couldn't allocate memory
			return false;
		}
		return insertVertex(v);
	}
	
	// find double the triangle area from three points
	int findTriangleArea2(PointInt &a, PointInt &b, PointInt &c)
	{
		return (b[X] - a[X]) * (c[Y] - a[Y]) - (c[X] - a[X]) * (b[Y] - a[Y]);
		//return (c[X] - b[X]) * (a[Y] - b[Y]) - (c[Y] - b[Y]) * (a[X] - b[X]);
	}

	// find double the polygon area
	int findPolygonArea2()
	{
		int sum = 0;
		Vertex *p, *a;
		p = head;
		a = p->next;

		do
		{
			// triangulate from the head as a centeral point
			sum += findTriangleArea2(p->v, a->v, a->next->v);
			a = a->next;
		} while (a->next != head);

		return sum;
	}

	// find if c is to the left of the line a-b
	bool isLeft(PointInt &a, PointInt &b, PointInt &c)
	{
		return findTriangleArea2(a, b, c) > 0;
	}

	// find if c is on the left or collinear with the line a-b
	bool isLeftOrEqual(PointInt &a, PointInt &b, PointInt &c)
	{
		int area = findTriangleArea2(a, b, c);
		//cout << area << endl;
		return area >= 0;
	}

	// find if c is collinear with the line a-b
	bool isCollinear(PointInt &a, PointInt &b, PointInt &c)
	{
		return findTriangleArea2(a, b, c) == 0;
	}

	// find if the line segments a-b and c-d intersect
	bool isIntersectWeak(PointInt &a, PointInt &b, PointInt &c, PointInt&d)
	{
		// first remove collinearity
		if (isCollinear(a, b, c) || isCollinear(a, b, d) || isCollinear(c, d, a) || isCollinear(c, d, b))
		{
			return 0;
		}
		if (isLeft(a, b, c) == isLeft(b, a, d))
		{
			if (isLeft(c, d, a) == isLeft(d, c, b))
			{
				return true;
			}
		}

		return false;
	}

	// find is the point c is collinear and between a and b
	bool isBetween(PointInt &a, PointInt &b, PointInt &c)
	{
		if (!isCollinear(a, b, c))
		{
			// if not collinear
			return false;
		}

		int minX = std::min(a[X], b[X]);
		int maxX = std::max(a[X], b[X]);
		int minY = std::min(a[Y], b[Y]);
		int maxY = std::max(a[Y], b[Y]);

		if (c[X] >= minX && c[X] >= maxX && c[Y] >= minY && c[Y] <= maxY)
		{
			return true;
		}

		return false;
	}

	// find if the line segments a-b and c-d intersect
	bool isIntersect(PointInt &a, PointInt &b, PointInt &c, PointInt&d)
	{
		if (!isIntersectWeak(a, b, c, d))
		{
			if (
				isBetween(a, b, c) ||
				isBetween(a, b, d) ||
				isBetween(c, d, a) ||
				isBetween(c, d, b))
			{
				return true;
			}
			return false;
		}
		return true;
	}

	// assuming a and b are not consecutive vertices, try to check if they can form a diagonal
	bool isDiagonalWeak(Vertex *a, Vertex *b)
	{
		Vertex *c, *c1;

		// check for all edges c-c1
		c = head;
		do
		{
			c1 = c->next;
			if (c != a && c1 != a && c != b && c1 != b && isIntersect(a->v, b->v, c->v, c1->v))
			{
				return false;
			}

			c = c1;

		} while (c != head);

		return true;
	}

	// is in cone (whether convex or concave)
	bool isInCone(Vertex *a, Vertex *b)
	{
		// a0, a, a1 are consecutive
		Vertex *a0, *a1;
		a1 = a->next;
		a0 = a->prev;

		// if convex
		if (isLeftOrEqual(a0->v, a->v, a1->v))
		{
			return (isLeft(a->v, b->v, a0->v) && isLeft(b->v, a->v, a1->v));
		}
		else
		{
			// reflex
			return ! (isLeftOrEqual(a->v, b->v, a1->v) && isLeftOrEqual(b->v, a->v, a0->v));
		}
	}

	// is this a diagonal
	bool isDiagonal(Vertex *a, Vertex *b)
	{
		return (isInCone(a, b) && isInCone(b, a) && isDiagonalWeak(a, b));
	}

	// set if each vertex is an ear tip
	void initEars()
	{
		Vertex *v0, *v1, *v2;

		v1 = head;
		do
		{
			v2 = v1->next;
			v0 = v1->prev;
			v1->ear = isDiagonal(v0, v2);
			v1 = v1->next;
		} while (v1 != head);
	}

	//triangulate 
	// each row is one triangle where the first and last vertices are the digonal
	vector<vector<Vertex*> > triangulate()
	{
		vector<vector<Vertex*> > res;
		Vertex *v[5];
		int n = nVertices;
		// init Ears
		initEars();
		int prevN = n + 1;
		while (n > 3 && prevN != n)
		{
			prevN = n;
			v[2] = head;
			do
			{
				if (v[2]->ear)
				{
					// init all vertices
					v[3] = v[2]->next; v[4] = v[3]->next;
					v[1] = v[2]->prev; v[0] = v[1]->prev;

					// v[1]->v[3] is a digonal
					vector<Vertex*> triangle;
					triangle.push_back(v[1]);
					triangle.push_back(v[2]);
					triangle.push_back(v[3]);
					res.push_back(triangle);
					// update ears
					v[1]->ear = isDiagonal(v[0], v[3]);
					v[3]->ear = isDiagonal(v[1], v[4]);

					// remove the current ear
					v[1]->next = v[3];
					v[3]->prev = v[1];
					head = v[3];
					n--;
					cout << n << endl;
					break;
				}
				v[2] = v[2]->next;

			} while (v[2] != head);
		}
		return res;
	}

	// draw
	void drawTriangulate()
	{
		cv::Mat img = cv::Mat::zeros(cv::Size(400, 400), CV_8UC3);

		Vertex*p = head;
		do
		{
			cv::line(img, cv::Point(p->v[0], p->v[1]), cv::Point(p->next->v[0], p->next->v[1]), cv::Scalar(255, 255, 255), 1);
			p = p->next;
		} while (p != head);
		vector<vector<Vertex*> > tris = triangulate();
		for (int i = 0; i < tris.size(); i++)
		{
			cv::line(img, cv::Point(tris[i][0]->v[0], tris[i][0]->v[1]), cv::Point(tris[i][2]->v[0], tris[i][2]->v[1]), cv::Scalar(0, 0, 255), 1);
		}
		cv::imshow("img", img);
		cv::waitKey();
	}

	// draw
	void draw()
	{
		cv::Mat img = cv::Mat::zeros(cv::Size(200, 200), CV_8UC3);

		Vertex*p = head;
		do
		{
			cv::line(img, cv::Point(p->v[0], p->v[1]), cv::Point(p->next->v[0], p->next->v[1]), cv::Scalar(255, 255, 255), 1);
			p = p->next;
		} while (p != head);

		cv::imshow("img", img);
		cv::waitKey();
	}

	void drawEars()
	{
		cv::Mat img = cv::Mat::zeros(cv::Size(200, 200), CV_8UC3);

		Vertex*p = head;
		int count = 0;
		do
		{
			//cv::line(img, cv::Point(p->v[0], p->v[1]), cv::Point(p->next->v[0], p->next->v[1]), cv::Scalar(255, 255, 255), 1);
			if (isDiagonal(p->prev, p->next))
			{
				cv::line(img, cv::Point(p->prev->v[0], p->prev->v[1]), cv::Point(p->next->v[0], p->next->v[1]), cv::Scalar(rand() % 256, rand() % 256, rand() % 256), 1);
				++count;
			}
			p = p->next;
		} while (p != head);
		cout << count << endl;
		cv::imshow("img", img);
		cv::waitKey();
	}
};

int main()
{
	cout << "Hello Geometry!" << endl;
	Polygon pol;
	//int x[] = { 1,1,-1,-1};
	//int y[] = { 1,-1, -1,  1};
	//int x[] = { -1,1,1,-1, 0};
	//int y[] = { -1,-1, 1, 1, 0};

	int x[] = { 0,10,12,20, 13, 10, 12, 14, 8,  6, 10,  7,  0,  1,  3, 5,-2, 5};
	int y[] = { 0, 7, 3, 8, 17, 12, 14, 9, 10, 14, 15, 18, 17, 13, 15, 8, 7, 5};
	
	//int x[] = { 0, 10, 3, 0};
	//int y[] = { 0, 0, 3, 10};
	
	for (int i = 0; i < 17; i++)
	{
		pol.insertVertex(PointInt({40 + 15 * x[i], 40 + 15 * y[i] }));
	}
	//pol.draw();
	//pol.drawEars();
	pol.drawTriangulate();
	
	return 0;
}