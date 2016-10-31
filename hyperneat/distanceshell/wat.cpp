#include <iostream>
#include <list>

#include <CGAL/AABB_intersections.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double> K;

typedef K::FT FT;
typedef K::Ray_3 Ray;
typedef K::Line_3 Line;
typedef K::Point_3 Point;
typedef K::Triangle_3 Triangle;

typedef std::list<Triangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<K,Iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fstream>
#include <vector>
using namespace std;


double norm(int r_xyVal, int r_numVoxelsXorY) 
{
    // turn the xth or yth node into its coordinates on a grid from -1 to 1, e.g. x values (1,2,3,4,5) become (-1, -.5 , 0, .5, 1)
    // this works with even numbers, and for x or y grids only 1 wide/tall, which was not the case for the original
    // e.g. see findCluster for the orignal versions where it was not a funciton and did not work with odd or 1 tall/wide #s
	
    double coord;
    
    if(r_numVoxelsXorY==1) coord = 0;
    else                  coord = -1 + ( r_xyVal * 2.0/(r_numVoxelsXorY-1) );
    
    return(coord);    
}

void StringExplode(string str, string separator, vector<string>* results){
    int found;
    found = str.find_first_of(separator);
    
    while(found != string::npos){
        if(found > 0){
            results->push_back(str.substr(0,found));
        }
        str = str.substr(found+1);
        found = str.find_first_of(separator);
    }
    
    if(str.length() > 0){
        results->push_back(str);
    }
}

//Turns foo into Point
//ACHEN: ADDED HACK, OUTPUTS TO STL
Point turncoords (string foo, float xoff, float yoff, float zoff, float denom, ofstream &output,string longestdim){
    vector<string> R;
    StringExplode(foo.substr(7,(foo.length()-7)), " ", &R);
    float a= atof(R[0].c_str());
    float b= atof(R[1].c_str());
    float c= atof(R[2].c_str());
    Point foo2;
    
    if (longestdim == "z"){
       foo2= Point((a + xoff)/denom, (c + zoff)/denom,(b + yoff)/denom); //Y AND Z ARE SWITCHED
    }
    else if (longestdim == "y"){
       foo2= Point((a + xoff)/denom,(b + yoff)/denom,(c + zoff)/denom);
    }
    else if (longestdim == "x"){
       foo2= Point((c + zoff)/denom,(a + xoff)/denom,(b + yoff)/denom);
    }
    
    //Write to new stl file
    output << "vertex " << (a + xoff)/denom << " " << (b + yoff)/denom << " " << (c + zoff)/denom << endl;
    
    return foo2;
}

//Similar to turncoords, but updates min and max values
void updatemaxes(string foo, vector<float> &xv, vector<float> &yv, vector<float> &zv){
   vector<string> R;
   StringExplode(foo.substr(7,(foo.length()-7))," ", &R);
   float x= atof(R[0].c_str());
   float y= atof(R[1].c_str());
   float z= atof(R[2].c_str());
    
	xv.push_back(x); //ACHEN: Might use arrays to be faster? @#
	yv.push_back(y);
	zv.push_back(z);
}

//Might be deprecated
float normalize(int min, int max, float num){
    float avg= (min + max)/2;
    if (num == avg) return 0;
    else if (num > avg) return (num - avg)/(max-avg);
    else return (num - min)/(avg-min) - 1;
}

int main (int argc, char * argv[]){
    
  	//Timing
  	time_t start,end;
  	time (&start);
    
  	Point a(0.0, 0.0, 0.0);
  	Point b(0.0, 0.0, 0.0);
  	Point c(0.0, 0.0, 0.0);
  	Point center(0.0, 0.0, 0.0);
   Point offcenter(0.5,0.5,0.5);
   Point randompoint(42.12134,45.324,65.2315);
    
  	std::list<Triangle> triangles;
    
   /** Strips leading spaces to avoid segfaults **/
	string argg(argv[1]);	
	string cmd= "perl -p -i -e 's/^[ \t]+//g' " + argg;
	int i= system(cmd.c_str());

  	ifstream file(argv[1]);
	ifstream file2(argv[1]);
	    
  	string line;
	string line2;
	
	int loop= 3;
	
	vector<float> xvector;
	vector<float> yvector;
	vector<float> zvector;
	
  	//First loop is for the normalization of the values
	while(getline(file2,line2)){
        if (loop < 3){
			switch (loop) {
				case 0:
               updatemaxes(line2, xvector, yvector, zvector);
               break;
                    
				case 1:
               updatemaxes(line2, xvector, yvector, zvector);
               break;
                    
				case 2:
				   updatemaxes(line2, xvector, yvector, zvector);
               break;
			}
			loop++;
        }
        if (line2.substr(0,5) == "outer"){
			loop=0;
        }
        if (line2.substr(0,8) == "endfacet"){
			loop= 3;
        }
	}
	file2.close();
	loop= 3;
	
	
	//Might want to switch to arrays later on, less costly
	float minx= *(min_element(xvector.begin(), xvector.end()));
	float miny= *(min_element(yvector.begin(), yvector.end()));
	float minz= *(min_element(zvector.begin(), zvector.end()));
	float maxx= *(max_element(xvector.begin(), xvector.end()));
	float maxy= *(max_element(yvector.begin(), yvector.end()));
	float maxz= *(max_element(zvector.begin(), zvector.end()));
	//printf("%f %f %f %f %f %f \n",minx,miny,minz,maxx,maxy,maxz);
	
	
	/** Normalization calculations **/
	
	//Offsets
	float xoff= -((minx + maxx)/2);
	float yoff= -((miny + maxy)/2);
	float zoff= -((minz + maxz)/2);
	
	//New min/max values after offsets
	float nminx= minx + xoff;
	float nmaxx= maxx + xoff;
	float nminy= miny + yoff;
	float nmaxy= maxy + yoff;
	float nminz= minz + zoff;
	float nmaxz= maxz + zoff;
	
   float final;
   string longestdim;
	
	/**
	if (nmaxz > nmaxy && nmaxz > nmaxx){
      final = (nmaxz/2); //Multiply by 1.x to get .x smaller shape
	}
	else{
   	final= max(max(nmaxy,nmaxz),nmaxx); //Might be a useless branch...
	}
	**/
	
	//WHICH DIMENSION NEEDS THE MOST SHRINKAGE
	
	//Determining longest dimension for rotation
	if (nmaxz > nmaxy && nmaxz > nmaxx){
	   final = (nmaxz/2); //Multiply by 1.x to get .x smaller shape
      longestdim= "z";
	}
	else if (nmaxx > nmaxy && nmaxx > nmaxz){
	   final = (nmaxx/2); //Multiply by 1.x to get .x smaller shape
      longestdim= "x";
	}
	else if (nmaxy > nmaxz && nmaxy > nmaxx){
	   final = (nmaxy/2); //Multiply by 1.x to get .x smaller shape
      longestdim= "y";
	}
	else{
	   final = (nmaxz/2); //Multiply by 1.x to get .x smaller shape
      longestdim= "z"; //Defaults to z if everything is equal
	}
	
	printf("%f %f %f %f %f %f \n",nminx,nminy,nminz,nmaxx,nmaxy,nmaxz);
	
	//STL file containing transformed shapes
	ofstream transformedstl("transformed.stl");
	
  	while(getline(file,line)){
      bool last= false;
		if (loop < 3){
		   switch (loop) {
				case 0:
               a= turncoords(line, xoff, yoff, zoff, final, transformedstl,longestdim);
               break;
                    
				case 1:
               b= turncoords(line, xoff, yoff, zoff, final, transformedstl,longestdim);
               break;
                    
				case 2:
               c= turncoords(line, xoff, yoff, zoff, final, transformedstl,longestdim);
               last= true;
               break;
            }
			loop++;
      }
      
      if (line.substr(0,5) == "outer"){
         transformedstl << line << endl;  
			loop=0;
      }
      else if (line.substr(0,8) == "endfacet"){
         transformedstl << line << endl;
			loop= 3;
         last= false;
			triangles.push_back(Triangle(a,b,c));
      }
      else{
         if (loop >= 3 && !last) transformedstl << line << endl;
      }
	}
	
  	// constructs AABB tree
  	Tree tree(triangles.begin(),triangles.end());
  	file.close();
    
    // counts #intersections
    /*
     Ray ray_query(a,b);
     std::cout << tree.number_of_intersected_primitives(ray_query)
     << " intersections(s) with ray query" << std::endl;
     */
    
    ofstream out("output.csv");
    ofstream pure("pure.csv");
    ofstream pure2("pure2.csv");
    
    out << "x,y,z,d,i" << endl;
    int dots=0;   
	 float z;
    float y;
	 float x;
	 float d;
    
    //Convert it so that its a different coordinate system? Rotate, basically
    for (int zi=0; zi<10; zi++) { //10
        z = -1 + zi * 2.0/9;
        for (int yi=0; yi<20; yi++){ //20 //Change normalized values to -2 to +2? y=y+.210526
            y = -2 + yi * 4.0/19;
            for (int xi=0; xi<10; xi++){ //10
                x = -1 + xi * 2.0/9;
                
                Point point_query(x, y, z);
                Point closest_point = tree.closest_point(point_query);
                FT sqd = tree.squared_distance(point_query);
		
                Ray rq1(point_query,randompoint);
                int foo= tree.number_of_intersected_primitives(rq1);
                
                d= sqrt(sqd); //Put outside
                
                //Mod 2 is outside
                
                //POINT IS OUTSIDE
                if (foo % 2 == 0){
                   d= -d; //ACHEN: TEMP HACK FOR VISUALIZATION
                }/**
                else{
                   Ray rq2(point_query,offcenter);
                   int foo2= tree.number_of_intersected_primitives(rq2);
                   if (foo2 % 2 == 0) d= -d;                   
                }**/
                
                out << x << "," << y << "," << z << "," << d << "," << foo << endl;
                pure << d << endl;
                pure2 << foo << endl; //In-out
                
                dots++;
            }
        }
    }
    
    out.close();
    pure.close();
    pure.close();
    
    time (&end);
    double dif = difftime (end,start);
    
    printf ("Operation took %.2lf seconds to complete.\n", dif );
    int ntriangles=triangles.size();
    printf ("Triangles in shape: %d\n",ntriangles);
    printf ("Points calculated: %d\n",dots);
    
    return EXIT_SUCCESS;
    
    
}
