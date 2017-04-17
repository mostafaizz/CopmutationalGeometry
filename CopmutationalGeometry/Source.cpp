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
	Vertex* head;
public:
	Polygon() { head = 0; }

	// inserting new vertex from Vertex
	bool insertVertex(Vertex *v)
	{
		if (head == 0)
		{
			head = v;
			head->vNum = 0;
			head->prev = head->next = head;
		}
		else
		{
			Vertex* tmp = head;
			while (tmp->next != head)
			{
				tmp = tmp->next;
			}
			v->next = head;
			head->prev = v;
			v->prev = tmp;
			tmp->next = v;
		}
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
		return findTriangleArea2(a, b, c) >= 0;
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


		do
		{
			cv::line(img, cv::Point(p->v[0], p->v[1]), cv::Point(p->next->v[0], p->next->v[1]), cv::Scalar(255, 255, 255), 1);
			if (!isDiagonal(p->prev, p->next))
			{
				cv::line(img, cv::Point(p->prev->v[0], p->prev->v[1]), cv::Point(p->next->v[0], p->next->v[1]), cv::Scalar(rand() % 256, rand() % 256, rand() % 256), 1);
			}
			p = p->next;
		} while (p != head);

		cv::imshow("img", img);
		cv::waitKey();
	}
};

int main()
{
	cout << "Hello Geometry!" << endl;
	Polygon pol;
	int x[] = { 10,10,-10,-10,0 };
	int y[] = { 10,-10, -10,  10, 0 };
	for (int i = 0; i < 5; i++)
	{
		pol.insertVertex(PointInt({ 100 + 3 * x[i], 100 + 3 * y[i] }));
	}
	pol.drawEars();
	return 0;
}