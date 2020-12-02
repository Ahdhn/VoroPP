//Build voronoi for a set of vertices.. Inspired by http://math.lbl.gov/voro++/examples/polygons/

#ifndef _VORO_PP_
#define _VORO_PP_

#include <voro++.hh>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <list>
#include <array>

class Voro
{
public:

	//Constructor with the container extends from (xmin, ymin, zmin) to (xmax, ymax, zmax) 	
	Voro(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)
		:m_xmin(xmin), m_xmax(xmax), m_ymin(ymin), m_ymax(ymax), m_zmin(zmin), m_zmax(zmax), m_is_computed(false) {};

	~Voro() {};


	//Builds voronoi diagram for the vertices inside the container
	template <typename T> //examples of T could be double**, std::vector<std::vector<double>>& or any similar type that can be indexed twice 
	void getVoronoi(int num_vertices, T& vertices);


	//Produces a .obj file of the voronoi faces 
	void drawVoronoi(std::string filename) const;

private:

	void build(voro::pre_container& pcon);

	int m_num_vertices;

	//define bounding box 
	const double m_xmin, m_xmax, m_ymin, m_ymax, m_zmin, m_zmax, ** vertices;

	//all voronoi faces 
	std::vector<std::vector<double>> m_voro_faces;

	//for each voronoi face, store the two seeds that created it
	std::vector< std::array<int, 2> >m_voro_neighbour;

	//for each seed, we store the starting and end id in
	//m_voro_faces that represent this seed cell 
	std::vector<std::vector<int>> m_voro_cells;

	//cache if voronoi has been computed 
	bool m_is_computed;

};

template <typename T>
void Voro::getVoronoi(int num_vertices, T& vertices_lambda)
{
	//Construct 3D voronoi using voro++ 
	//initiate the voro++ container and pass it to the build function 
	//check if all vertices are inside the main bounding box 

	m_num_vertices = num_vertices;
	//1) init container 
	//use a pre container to get the optimal number of blocks that the container is divided into 	
	voro::pre_container pcon(m_xmin, m_xmax, m_ymin, m_ymax, m_zmin, m_zmax, false, false, false);

	//2) add vertices to the pre container (it will be passed then to the main container)	
	for (int i = 0; i < num_vertices; i++) {
		double x(vertices_lambda(i, 0)), y(vertices_lambda(i, 1)), z(vertices_lambda(i, 2));
		if (x < m_xmin || x > m_xmax || y < m_ymin || y > m_ymax || z < m_zmin || z > m_zmax) {
			std::cout << "ERROR::Voro::getVoronoi():: Vertex not inside the bounding box. It will be skipped!!!" << std::endl;
			std::cout << x << ", " << y << ", " << z << std::endl;
			continue;
		}
		pcon.put(i, x, y, z);

		std::vector<int> cell_id(2);
		m_voro_cells.push_back(cell_id);
	}

	//3)call the builder 
	build(pcon);

	m_is_computed = true;

}

void Voro::build(voro::pre_container& pcon)
{

	//do the computation
	//instead of calling compute_all (which can do all the work)
	//we implement the compute_all here so we can access each cell 
	//and so we don't have to allocate additional memory for each cell 
	//and have access to all voronoi vertices and faces

	int nx, ny, nz;
	pcon.guess_optimal(nx, ny, nz); //get optimal number of blocks 

	voro::container con(m_xmin, m_xmax, m_ymin, m_ymax, m_zmin, m_zmax, nx, ny, nz, false, false, false, 8); //1st to 6th paramter is the containers 
	pcon.setup(con);//pass the vertices to the (main) container									 //7th to 9th parameter is the number of blocks that the container is divided into 
	//10th to 12 is isPeriodic in x, y or z 
	//13th parameter is allocation space for how many particles within each computational block 
	voro::voronoicell_neighbor cn;																 //(this is just for guidance, the code will dynamically allocate more in case of overflow)	
	voro::c_loop_all cl(con);


	double x, y, z; //my current cell center
	int id;//my current cell id


	size_t num_processed = 0;

	std::cout << "Voro::build 0% ";
	std::vector<int> percentage(1, 0);

	//loop over all vertices and compute their cells 
	if (cl.start())do if (con.compute_cell(cn, cl)) {

		cl.pos(x, y, z);//get the position of the current particle 
		id = cl.pid();//get its id (FLAG THIS)
		num_processed++;


		//std::cout << "num_processed= " << num_processed << std::endl;

		int percetnage_processed = int(100 * double(num_processed) / double(m_num_vertices - 1));
		if (percetnage_processed % 10 == 0
			&& percetnage_processed != percentage.back()) {
			std::cout << percetnage_processed << "%  ";
			percentage.push_back(percetnage_processed);
		}


		std::vector<int> neigh, f_vert;//my current cell list of neighbours and faces 
		std::vector <double>v;//my current cell's vertices vector 						

		cn.neighbors(neigh);//list of neighbours		
		cn.face_vertices(f_vert);//list of the number of vertices per face 		                         
		cn.vertices(x, y, z, v);//get/build its voronoi vertices 		


		//loop over all faces of the voronoi cell 
		//extract the faces that separate in/out vertices
		m_voro_cells[id][0] = m_voro_faces.size();

		for (size_t i = 0, j = 0; i < neigh.size(); i++, j += f_vert[j] + 1) {
			if (neigh[i] > 0) {

				//skip if the neighbour id is smaller than my id
				//to avoid double counting (also this removes faces 
				//that touch the  container boundary

				//this loops over the face vertices such that 
				//x= v[my_ver_id], y=v[my_ver_id+1], z=[my_ver_id+2]
				//for each vertex and number of vertices = f_vert[j]

				std::vector<double> my_face;
				for (size_t k = j + 1; k <= j + f_vert[j]; k++) {
					size_t my_ver_id = 3 * f_vert[k];
					//record face vertices here 
					my_face.push_back(v[my_ver_id]);
					my_face.push_back(v[my_ver_id + 1]);
					my_face.push_back(v[my_ver_id + 2]);
				}
				m_voro_faces.push_back(my_face);

				std::array<int, 2> neighbours{ id, i };
				m_voro_neighbour.push_back(neighbours);

			}
		}
		m_voro_cells[id][1] = m_voro_faces.size();


	}while (cl.inc());

	std::cout << "  DONE!" << std::endl;
}


void Voro::drawVoronoi(std::string filename) const
{
	//draw the voronoi faces (crust). should be called after build
	if (!m_is_computed) {
		std::cout << "ERROR::Voro::drawVoronoi():: There is no faces to draw. Call getVoronoi() first!!!" << std::endl;
		return;
	}
	std::fstream file(filename, std::ios::out);
	file.precision(20);
	size_t total_vertices(1);

	//draw vertices followed by the face of voro_faces 
	for (size_t i = 0; i < m_voro_faces.size(); i++) {
		//each face (voro_faces[i]) is stored as x,y,z of the point composes it 
		for (size_t v = 0; v < m_voro_faces[i].size(); v += 3) {
			file << "v	" << m_voro_faces[i][v] << "	" << m_voro_faces[i][v + 1]
				<< "	" << m_voro_faces[i][v + 2] << std::endl;
		}

		file << "f	";
		for (size_t v = 0; v < m_voro_faces[i].size() / 3; v++) {
			file << v + total_vertices << "		";
		}
		file << std::endl;
		total_vertices += m_voro_faces[i].size() / 3;
	}
}

#endif /*_VORO_PP_*/