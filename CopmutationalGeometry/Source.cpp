#include <iostream>
#include <vector>

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
	bool insertVertex(PointInt pt)
	{
		Vertex * v = new Vertex;
		v->v = pt;
		if (v == 0)
		{// couldn't allocate memory
			return false;
		}
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
		return false;
	}
	
	int findTriangleArea2(PointInt &a, PointInt &b, PointInt &c)
	{
		return (b[X] - a[X]) * (c[Y] - a[Y]) - (c[X] - a[X]) * (b[Y] - a[Y]);
	}

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
	bool isIntersect(PointInt &a, PointInt &b, PointInt &c, PointInt&d)
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
};

int main()
{
	cout << "Hello Geometry!" << endl;

	return 0;
}