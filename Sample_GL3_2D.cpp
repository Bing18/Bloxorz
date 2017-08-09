#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <math.h>
#include <stdlib.h>//for random number
#include <time.h> //for time

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <thread>
#include <ao/ao.h>
#include <mpg123.h>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;
int  fbwidth=1200,fbheight=800;
struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
  glm::mat4 view1;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
typedef struct Floor {
  GLfloat floor_vertex_buffer [108] = {
    //LEFT FACE 
    -0.25f,-0.1f,-0.25f,
    -0.25f,-0.1f,0.25f,
    -0.25f,0.1f,0.25f,

    -0.25f,-0.1f,-0.25f,
    -0.25f,0.1f,0.25f,
    -0.25f,0.1f,-0.25f,

    //RIGHT FACE
    0.25f,0.1f,0.25f,
    0.25f,-0.1f,-0.25f,
    0.25f,0.1f,-0.25f,

    0.25f,-0.1f,-0.25f,
    0.25f,0.1f,0.25f,
    0.25f,-0.1f,0.25f,

    //TOP FACE
    0.25f,0.1f,0.25f,
    0.25f,0.1f,-0.25f,
    -0.25f,0.1f,-0.25f,
    
    0.25f,0.1f,0.25f,
    -0.25f,0.1f,-0.25f,
    -0.25f,0.1f,0.25f,

    //Bottom FACE
    0.25f,-0.1f,0.25f,
    -0.25f,-0.1f,-0.25f,
    0.25f,-0.1f,-0.25f,

    0.25f,-0.1f,0.25f,
    -0.25f,-0.1f,0.25f,
    -0.25f,-0.1f,-0.25f,

    //FRONT FACE
    -0.25f,0.1f,0.25f,
    -0.25f,-0.1f,0.25f,
    0.25f,-0.1f,0.25f,

    0.25f,0.1f,0.25f,
    -0.25f,0.1f,0.25f,
    0.25f,-0.1f,0.25f,

    //BACK FACE 
    0.25f,0.1f,-0.25f,
    -0.25f,-0.1f,-0.25f,
    -0.25f,0.1f,-0.25f,
    
    0.25f,0.1f,-0.25f,
    0.25f,-0.1f,-0.25f,
    -0.25f,-0.1f,-0.25f
    

  };

  GLfloat floor_color_buffer [108];
  int status;
  float x,y,z;
  VAO* Object;
}Floor_struct;

struct COLOR//for colors 
{
  float r,g,b;
};
typedef struct COLOR COLOR;

typedef struct Things {
  float x,y,z;
  VAO* Object;
  int flag;
  COLOR color;
  float radius,height,width;
  int status;
}Things;


Floor_struct Floor[10][15];
Things circle1[100]; 
Things cube1,cube2;

float base_x,base_y;
//for points
string scoreLabel="POINTS";
Things scoreLabelObjects[12],score_valueObjects[12];//score label;
float scoreLabel_x=2,scoreLabel_y=5.5;
float score_value_x=4.7,score_value_y=5.5;
int cur_score=0,prev_score=0,win_score=120;
//for level 
string levelLabel="STAGE";
Things levelLabelObjects[12],level_valueObjects[12];//score label;
float levelLabel_x=-5.7,levelLabel_y=-5.5;
float level_value_x=-3.5,level_value_y=-5.5;


//for GAME OVER
Things endLabelObjects[12]; //The you win/lose label
string endLabel="";
float endLabel_x=-3,endLabel_y=0;

int right_rotate=0;
int cur_level=1;
float level_x_trans,level_y_trans,level_z_trans;
float cube_x_trans=0,cube_y_trans=0,cube_z_trans=0;
float utime=glfwGetTime(),cur_time;
float cube_falling_anim=15;
float floor_falling_anim=-15;

int bridge1=0,bridge2=0,bridge1_1=0,bridge2_1=0,teleport=0,combined_flag=1;

float x_change=0,y_change=0;//for camera pan
float zoom_camera=1;//for zooming camera

int disable_controls=0;
int game_over_var=0;
int v=1;

//mouse controls
int right_mouse_clicked=0;
double new_mouse_pos_x, new_mouse_pos_y;
double mouse_pos_x, mouse_pos_y;
int m_flag0=0,m_flag1=0,m_flag2=0;
double mouse_x,mouse_y,m_click_x;

float tower_eye_x,tower_eye_y,tower_eye_z;

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/*SOUND*/
void* play_audio(string audioFile){
  mpg123_handle *mh;
  unsigned char *buffer;
  size_t buffer_size;
  size_t done;
  int err;

  int driver;
  ao_device *dev;

  ao_sample_format format;
  int channels, encoding;
  long rate;

  /* initializations */
  ao_initialize();
  driver = ao_default_driver_id();
  mpg123_init();
  mh = mpg123_new(NULL, &err);
  buffer_size = mpg123_outblock(mh);
  buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

  /* open the file and get the decoding format */
  mpg123_open(mh, &audioFile[0]);
  mpg123_getformat(mh, &rate, &channels, &encoding);

  /* set the output format and open the output device */
  format.bits = mpg123_encsize(encoding) * 8;
  format.rate = rate;
  format.channels = channels;
  format.byte_format = AO_FMT_NATIVE;
  format.matrix = 0;
  dev = ao_open_live(driver, &format, NULL);

  /* decode and play */
  char *p =(char *)buffer;
  while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
    ao_play(dev, p, done);

  /* clean up */
  free(buffer);
  ao_close(dev);
  mpg123_close(mh);
  mpg123_delete(mh);
}


void check_pan()
{
  if(x_change-6.0f/zoom_camera<-6)
        x_change=-6+6.0f/zoom_camera;
  else if(x_change+6.0f/zoom_camera>6)
        x_change=6-6.0f/zoom_camera;
  if(y_change-6.0f/zoom_camera<-6)
        y_change=-6+6.0f/zoom_camera;
  else if(y_change+6.0f/zoom_camera>6)
        y_change=6-6.0f/zoom_camera;
}
void mousescroll(GLFWwindow* window,double xoffset, double yoffset)
{
    if (yoffset==-1) {
        zoom_camera /= 1.1; //make it bigger than current size
    }
    else if(yoffset==1){
        zoom_camera *= 1.1; //make it bigger than current size
    }
    if (zoom_camera<=1) {
        zoom_camera = 1;
    }
    if (zoom_camera>=4) {
        zoom_camera=4;
    }
    check_pan();
    Matrices.projection = glm::ortho(-6.0f/zoom_camera, 6.0f/zoom_camera, -6.0f/zoom_camera, 6.0f/zoom_camera, 0.1f, 500.0f);
}

void move(int i)
{
  if(i==1)
      {
        cur_score++;
        if(teleport==0)
        {
        if(fabs(cube1.x-cube2.x)<0.00001 && fabs(cube1.y-cube2.y)<0.00001)
        {
          cube1.x+=0.5;cube2.x+=0.5;
        }
        else if(fabs(cube1.x-cube2.x)<0.00001 && cube1.y<cube2.y)
        {
          cube1.x+=0.5;cube2.x+=1.0;cube2.y-=0.5;
        }
        else if(fabs(cube1.x-cube2.x)<0.00001 && cube1.y>cube2.y)
        {
          cube2.x+=0.5;cube1.x+=1.0;cube1.y-=0.5;
        }
        else if(cube1.x>cube2.x && fabs(cube1.y-cube2.y)<0.00001)
        {
          cube1.x+=0.5;cube2.x+=1.0;cube2.y+=0.5;
        }
        else if(cube1.x<cube2.x && fabs(cube1.y-cube2.y)<0.00001)
        {
          cube2.x+=0.5;cube1.x+=1.0;cube1.y+=0.5;
        }
      }
      else if(teleport==1)
      {
        cube2.x+=0.5;
      }
      else if(teleport==2)
      {
        cube1.x+=0.5;
      }

        
      }
      else if(i==2)
      {
        cur_score++;
        if(teleport==0)
        {
        if(fabs(cube1.x-cube2.x)<0.00001 && fabs(cube1.y-cube2.y)<0.00001)
        {
          cube1.x-=0.5;cube2.x-=0.5;
        }
        else if(fabs(cube1.x-cube2.x)<0.00001 && cube1.y<cube2.y)
        {
          cube1.x-=0.5;cube2.x-=1.0;cube2.y-=0.5;
        }
        else if(fabs(cube1.x-cube2.x)<0.00001 && cube1.y>cube2.y)
        {
          cube2.x-=0.5;cube1.x-=1.0;cube1.y-=0.5;
        }
        else if(cube1.x>cube2.x && fabs(cube1.y-cube2.y)<0.00001)
        {
          cube2.x-=0.5;cube1.x-=1.0;cube1.y+=0.5;
        }
        else if(cube1.x<cube2.x && fabs(cube1.y-cube2.y)<0.00001)
        {
          cube1.x-=0.5;cube2.x-=1.0;cube2.y+=0.5;
        }
      }
        else if(teleport==1)
      {
        cube2.x-=0.5;
      }
      else if(teleport==2)
      {
        cube1.x-=0.5;
      }
        
      }
      else if(i==3)
      {
        cur_score++;
        if(teleport==0)
        {
        if(fabs(cube1.z-cube2.z)<0.00001 && fabs(cube1.y-cube2.y)<0.00001)
          {cube1.z-=0.5;cube2.z-=0.5;}
        else if(fabs(cube1.z-cube2.z)<0.00001 && cube1.y<cube2.y)
          {cube1.z-=0.5;cube2.z-=1.0;cube2.y-=0.5;} 
        
        else if(fabs(cube1.z-cube2.z)<0.00001 && cube1.y>cube2.y)
          {cube1.z-=1.0;cube2.z-=0.5;cube1.y-=0.5;}
        
        else if(cube1.z>cube2.z && fabs(cube1.y-cube2.y)<0.00001)
          {cube1.z-=1.0;cube2.z-=0.5;cube1.y+=0.5;}
        
        else if(cube1.z<cube2.z && fabs(cube1.y-cube2.y)<0.00001)
          {cube1.z-=0.5;cube2.z-=1.0;cube2.y+=0.5;}
        }
        else if(teleport==1)
        {
          cube2.z-=0.5;
        }
        else if(teleport==2)
        {
        cube1.z-=0.5;
        }
      }
      else if(i==4)
      {
        cur_score++;
        if(teleport==0)
        {
        if(fabs(cube1.z-cube2.z)<0.00001 && fabs(cube1.y-cube2.y)<0.00001)
          {cube1.z+=0.5;cube2.z+=0.5;}
        else if(fabs(cube1.z-cube2.z)<0.00001 && cube1.y<cube2.y)
          {cube1.z+=0.5;cube2.z+=1.0;cube2.y-=0.5;}
        
        else if(fabs(cube1.z-cube2.z)<0.00001 && cube1.y>cube2.y)
          {cube1.z+=1.0;cube2.z+=0.5;cube1.y-=0.5;}
        
        else if(cube1.z>cube2.z && fabs(cube1.y-cube2.y)<0.00001)
          {cube1.z+=0.5;cube2.z+=1.0;cube2.y+=0.5;}
        
        else if(cube1.z<cube2.z && fabs(cube1.y-cube2.y)<0.00001)
          {cube1.z+=1.0;cube2.z+=0.5;cube1.y+=0.5;}
        }
        else if(teleport==1)
      {
        cube2.z+=0.5;
      }
      else if(teleport==2)
      {
        cube1.z+=0.5;
      }
}
}
float camera_rotation_angle = 120;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
    if (action == GLFW_REPEAT || action == GLFW_PRESS) 
    {

      if(key==GLFW_KEY_RIGHT && !disable_controls)
      {
        if(v==4 || v==5)
        {
          move(4);
        }
        else
        {
        move(1);
        }
      }
      else if(key==GLFW_KEY_LEFT && !disable_controls)
      {
        if(v==4 || v==5)
        {
          move(3);
        }
        else
        {

        move(2);
        }        
      }
      else if(key==GLFW_KEY_UP && !disable_controls)
      {
        if(v==4 || v==5)
        {
          move(1);
        }
        else
          move(3);
      }
      else if(key==GLFW_KEY_DOWN && !disable_controls)
      {if(v==4 || v==5)
        {
          move(2);
        }
        else
        move(4);        
      }
      else if(key==GLFW_KEY_1)
          {
            v=1;
            camera_rotation_angle=120;
          }
      else if(key==GLFW_KEY_2)
          v=2;
      else if(key==GLFW_KEY_3)
          {
            tower_eye_x=5*cos(120*M_PI/180.0f);
            tower_eye_y=7;
            tower_eye_z=5*sin(120*M_PI/180.0f);
            v=3;
          }
      else if(key==GLFW_KEY_4)
      {
        tower_eye_x=0;
            tower_eye_y=0;
            tower_eye_z=0;
            camera_rotation_angle=0;
          v=4;
      }
      else if(key==GLFW_KEY_5)
         {
          tower_eye_x=0;
            tower_eye_y=0;
            tower_eye_z=0;
            camera_rotation_angle=0;

          v=5;
         }
      

       else if(key==GLFW_KEY_O  && !disable_controls) //for zoom in
              {
                mousescroll(window,0,1);
                check_pan();
              }
        else if(key==GLFW_KEY_P && !disable_controls) //for zoom out
                {
                mousescroll(window,0,-1);
                check_pan();
                }
        else if(key==GLFW_KEY_A && !disable_controls) //for panning left
                {//flag_left=0;
                x_change-=1;
                mousescroll(window,0,0);
                //check_pan();
                }
        else if(key==GLFW_KEY_D && !disable_controls) //for panning rigth
                {//flag_right=0;
                x_change+=1;
                mousescroll(window,0,0);
                } 
        else if(key==GLFW_KEY_W && !disable_controls) //for panning up
                {//flag_right=0;
                y_change+=1;
                mousescroll(window,0,0);
                }
        else if(key==GLFW_KEY_S && !disable_controls) //for panning down
                {//flag_right=0;
                y_change-=1;
                mousescroll(window,0,0);
                }

    }
    if(action ==GLFW_RELEASE)
    {
      if(key==GLFW_KEY_RIGHT)
      {
       
      }
    }
    
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            exit(0);
            break;
		default:
			break;
	}
}
int m_flag=0,flag3=0,flag2=0,right_clicked=0;
float m1_x,m1_z,m2_x,m2_z;
float m1x,m1y;
static void cursor_position(GLFWwindow* window, double xpos, double ypos)
{
  mouse_x= ((12*xpos)/fbwidth)-6;
  mouse_y=-((12*ypos)/fbheight)+6;
  //cout<<"mouse_x:"<<mouse_y<<endl;
  printf("%lf %lf %lf %lf %lf %lf\n",cube1.x+cube_x_trans-0.25,mouse_x,cube1.x+cube_x_trans+0.25,cube1.y+cube_y_trans-0.5,mouse_y,cube1.y+cube_y_trans+0.5);
  
}
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  if(action == GLFW_RELEASE){
    if(button == GLFW_MOUSE_BUTTON_LEFT){
      if(m_flag==1){
        m2_x=mouse_x;
        m2_z=mouse_y;
        flag2++;
      }
      m_flag=0;
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT){
      right_clicked=0;
    }
  }

  else if(action == GLFW_PRESS){
    if(button == GLFW_MOUSE_BUTTON_LEFT){
      if((mouse_x>=cube1.x+cube_x_trans-0.25 && mouse_x<=cube1.x+cube_x_trans+0.25 && 
        mouse_y>=cube1.y+cube_y_trans-0.5 && mouse_y<=cube1.y+cube_y_trans+0.5)||
        (mouse_x<=cube2.x+cube_x_trans-0.25 && mouse_x>=cube2.x+cube_x_trans+0.25 && 
          mouse_y<=cube2.y+cube_y_trans-0.5 && mouse_y>=cube2.y+cube_y_trans+0.5))
      {
        //printf("asd\n");
        if(m_flag==0){
          m1_x=mouse_x;
          m1_z=mouse_y;
          flag2++;
        }
        m_flag=1;
      }
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT){
      if(right_clicked==0){
        right_clicked=1;
        m1x=mouse_x;
        m1y=mouse_y;
      }
    }
  }
    
}
void game_over()
{
  game_over_var=1;
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-6.0f/zoom_camera, 6.0f/zoom_camera, -6.0f/zoom_camera, 6.0f/zoom_camera, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle (string name,int weight,COLOR v1,COLOR v2,COLOR v3,COLOR v4,float x,float y,float height,float width,string component)
{
  // GL3 accepts only Triangles. Quads are not supported
  float w=width/2;
  float h=height/2;
  GLfloat vertex_buffer_data [] = {
    -w,-h,0, // vertex 1
    -w,h,0, // vertex 2
    w,h,0, // vertex 3

    w, h,0, // vertex 3
    w, -h,0, // vertex 4
    -w,-h,0  // vertex 1
  };

   GLfloat color_buffer_data [] = {
     v1.r,v1.g,v1.b,
     v2.r,v2.g,v2.b,
     v3.r,v3.g,v3.b,
     v3.r,v3.g,v3.b,
     v4.r,v4.g,v4.b,
     v1.r,v1.g,v1.b
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

  Things temp={};
  //temp.name=name;
  temp.color=v1;
  //temp.number=weight;
  temp.Object=rectangle;
  temp.x=x;
  temp.y=y;
  temp.height=height;
  temp.width=width;
  //temp.radius=(sqrt(height*height+width*width))/2;
  
  if(component=="scorelabel")
  {
    scoreLabelObjects[weight]=temp;
  }
  else if(component=="levellabel")
  {
    levelLabelObjects[weight]=temp;
  }
  else if(component=="endlabel")
  {
    endLabelObjects[weight]=temp;
  }
  else if(component=="score_value")
  {
    score_valueObjects[weight]=temp;
  }
  else if(component=="level_value")
  {
    level_valueObjects[weight]=temp;
  }
}

void createCircle (int weight,COLOR c1,float x,float y,float z,float radius,float parts,int fill) //c1,c2,c3 are colors
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
  int temp1=(int)360*parts;
  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  GLfloat vertex_buffer_data [9*temp1]={0};
  for(int i=0;i<temp1;i++)
  {
    vertex_buffer_data[9*i]=0;
    vertex_buffer_data[9*i+1]=0;
    vertex_buffer_data[9*i+2]=0;
    vertex_buffer_data[9*i+3]=radius*cos(i*M_PI/180);
    vertex_buffer_data[9*i+4]=0;
    vertex_buffer_data[9*i+5]=radius*sin(i*M_PI/180);
    vertex_buffer_data[9*i+6]=radius*cos((i+1)*M_PI/180);
    vertex_buffer_data[9*i+7]=0;
    vertex_buffer_data[9*i+8]=radius*sin((i+1)*M_PI/180);
  }

   GLfloat color_buffer_data [9*temp1];
   
  
  for (int i = 0; i<9*temp1 ; i+=3)
  {
    color_buffer_data[i]=c1.r;
    color_buffer_data[i+1]=c1.g;
    color_buffer_data[i+2]=c1.b;
  }
  VAO* circle;
  if(fill==1)
        circle = create3DObject(GL_TRIANGLES, 3*temp1, vertex_buffer_data, color_buffer_data, GL_FILL);
  else
        circle = create3DObject(GL_TRIANGLES, 3*temp1, vertex_buffer_data, color_buffer_data, GL_LINE);
  Things temp={};
  //temp.name=name;
  temp.color=c1;
  //temp.number=weight;
  temp.Object=circle;
  temp.x=x;
  temp.y=y;
  temp.z=z;
  
  temp.radius=radius;
  circle1[weight]=temp;
}

void create_floor()
{
  int i,j,k;
  for(i=0;i<10;i++)
  {
    for(j=0;j<15;j++)
    {
      Floor[i][j].x=0.5*j-1.5;
      Floor[i][j].z=0.5*i-1.5;
      Floor[i][j].y=0;

      for(k=0;k<36;k++)
      {
        Floor[i][j].floor_color_buffer[3*k]=0;
        Floor[i][j].floor_color_buffer[3*k+1]=0;
        Floor[i][j].floor_color_buffer[3*k+2]=0;

      }
      
      
      for(k=12;k<18;k++)
      {
        if(k%3==0)
        {Floor[i][j].floor_color_buffer[3*k]=1;
        }
        if(k%15==0)
         {Floor[i][j].floor_color_buffer[3*k]=0.9;
        } 
      }
  }
  }
  
  for(i=0;i<10;i++)
  {
    for(j=0;j<15;j++)
    {
      Floor[i][j].status=0;
    }
  }
  /*COMMON FOR ALL LEVELS*/
  cube_falling_anim=15;
  floor_falling_anim=-15;

  //level 1
  /*
  DEAD=>status=0;
  NORMAL=>status=1;
  FRAGILE=>status=2;
  SINGLE_SWITCH=>status=3;
  GOAL=>status=4;
  TELEPORT=>status=5;
  DOUBLE_SWITCH=>status=6;
  */
  cur_score=prev_score;
  if(cur_level==1)
  {
      for(i=0;i<10;i++)
      {
        if(i==0){for(j=0;j<3;j++){Floor[i][j].status=1;}}
        else if(i==1){for(j=0;j<6;j++){Floor[i][j].status=1;}} 
        else if(i==2){for(j=0;j<9;j++){Floor[i][j].status=1;}}
        else if(i==3){for(j=1;j<10;j++){Floor[i][j].status=1;}}
        else if(i==4){for(j=5;j<10;j++){Floor[i][j].status=1;}}
        else if(i==5){for(j=6;j<9;j++){Floor[i][j].status=1;}}   
      }
      Floor[4][7].status=4;
      for(i=0;i<10;i++)
      {
        //if(i==0){for(j=0;j<3;j++){Floor[i][j].status=1;}}
        if(i==1){for(j=5;j<6;j++){Floor[i][j].status=2;}} 
        else if(i==2){for(j=4;j<7;j++){Floor[i][j].status=2;}}
        else if(i==3){for(j=4;j<7;j++){Floor[i][j].status=2;}}
        else if(i==4){Floor[i][5].status=2;Floor[i][9].status=2;}
        else if(i==5){Floor[i][7].status=2;}   
      }
      level_x_trans=0;
      level_y_trans=0;
      level_z_trans=0;
      cube_x_trans=0;
      cube_y_trans=0;
      cube_z_trans=0;
  }

  //level 2
  if(cur_level==2)
  {
      bridge1=0,bridge2=0,bridge1_1=0,bridge2_1=0;
      //bridge2=0;
      //bridge1=0;//NOT PRESENT
      for(i=0;i<10;i++)
      {
        if(i==0){for(j=6;j<10;j++){Floor[i][j].status=1;}for(j=12;j<15;j++){Floor[i][j].status=1;}  }
        else if(i==1){for(j=0;j<4;j++){Floor[i][j].status=1;}for(j=6;j<10;j++){Floor[i][j].status=1;}for(j=12;j<15;j++){Floor[i][j].status=1;}} 
        else if(i==2){for(j=0;j<4;j++){Floor[i][j].status=1;}for(j=6;j<10;j++){Floor[i][j].status=1;}for(j=12;j<15;j++){Floor[i][j].status=1;}}
        else if(i==3){for(j=0;j<4;j++){Floor[i][j].status=1;}for(j=6;j<10;j++){Floor[i][j].status=1;}for(j=12;j<15;j++){Floor[i][j].status=1;}}
        else if(i==4){for(j=0;j<4;j++){Floor[i][j].status=1;}for(j=6;j<10;j++){Floor[i][j].status=1;}for(j=12;j<15;j++){Floor[i][j].status=1;}}
        else if(i==5){for(j=0;j<4;j++){Floor[i][j].status=1;}for(j=6;j<10;j++){Floor[i][j].status=1;}}
      }
      Floor[1][13].status=4;
      Floor[1][8].status=6;//DOUBLE SWITCH
      Floor[2][2].status=3;//SINGLE SWITCH
      for(i=0;i<10;i++)
      {
        if(i==0){Floor[i][6].status=2;Floor[i][9].status=2;Floor[i][12].status=2;Floor[i][14].status=2;}
        else if(i==1){Floor[i][7].status=2;Floor[i][0].status=2;Floor[i][3].status=2;} 
        else if(i==4){Floor[i][3].status=2;Floor[i][9].status=2;Floor[i][13].status=2;}
        else if(i==5){Floor[i][7].status=2;Floor[i][1].status=2;Floor[i][2].status=2;}   
      }
      level_x_trans=-1;
      level_y_trans=0;
      level_z_trans=-1;
      cube_x_trans=-1;
      cube_y_trans=0;
      cube_z_trans=0.5;

      

  }
  if(cur_level==3)
  {

    for(i=0;i<10;i++)
      {
        if(i==0){for(j=6;j<=12;j++){Floor[i][j].status=1;}}  
        else if(i==1){for(j=0;j<4;j++){Floor[i][j].status=1;}for(j=6;j<9;j++){Floor[i][j].status=1;}for(j=11;j<13;j++){Floor[i][j].status=1;}} 
        else if(i==2){for(j=0;j<9;j++){Floor[i][j].status=1;}for(j=11;j<15;j++){Floor[i][j].status=1;}}
        else if(i==3){for(j=0;j<4;j++){Floor[i][j].status=1;}for(j=11;j<15;j++){Floor[i][j].status=1;}}
        else if(i==4){for(j=0;j<4;j++){Floor[i][j].status=1;}for(j=12;j<15;j++){Floor[i][j].status=1;}}
      }
      Floor[3][13].status=4;
      Floor[0][11].status=2;
      Floor[2][3].status=2;
      Floor[2][13].status=2;
      level_x_trans=-1;
      level_y_trans=0;
      level_z_trans=-1+0.5;
      cube_x_trans=-1;
      cube_y_trans=0;
      cube_z_trans=0.5;      

  }
  if(cur_level==4)
  {
    bridge1=0,bridge2=0,bridge1_1=0,bridge2_1=0;
    for(i=0;i<10;i++)
      {
        if(i==0){for(j=3;j<13;j++){Floor[i][j].status=1;}}
        else if(i==1){for(j=3;j<13;j++){Floor[i][j].status=1;}} 
        else if(i==2){for(j=0;j<4;j++){Floor[i][j].status=1;}for(j=10;j<15;j++){Floor[i][j].status=1;}}
        else if(i==3){for(j=0;j<3;j++){Floor[i][j].status=1;}for(j=10;j<15;j++){Floor[i][j].status=1;}}
        else if(i==4){for(j=0;j<3;j++){Floor[i][j].status=1;}for(j=10;j<15;j++){Floor[i][j].status=1;}}
        else if(i==5){for(j=0;j<3;j++){Floor[i][j].status=1;}for(j=5;j<15;j++){Floor[i][j].status=1;}}
        else if(i==6){for(j=0;j<3;j++){Floor[i][j].status=1;}for(j=5;j<15;j++){Floor[i][j].status=1;}}
        else if(i==7){for(j=5;j<8;j++){Floor[i][j].status=1;}for(j=10;j<15;j++){Floor[i][j].status=1;}}
        else if(i==8){for(j=5;j<8;j++){Floor[i][j].status=1;}for(j=10;j<15;j++){Floor[i][j].status=1;}}
      }
      for(i=0;i<10;i++) 
       {if(i==6){for(j=5;j<8;j++){Floor[i][j].status=2;}}
        else if(i==7){for(j=5;j<8;j++){Floor[i][j].status=2;}}
        else if(i==8){for(j=5;j<8;j++){Floor[i][j].status=2;}}
        }
      Floor[7][13].status=6;
      Floor[7][6].status=4;
      Floor[5][9].status=0;
      Floor[6][9].status=0;
      Floor[5][10].status=2;
      Floor[2][10].status=2;
      Floor[2][1].status=2;
      Floor[2][2].status=2; 
      
      level_x_trans=-1;
      level_y_trans=0;
      level_z_trans=-1;
      cube_x_trans=-1;
      cube_y_trans=0;
      cube_z_trans=1;    

  }
  if(cur_level==5)
  {
    //bridge1=0,bridge2=0,bridge1_1=0,bridge2_1=0;
    teleport=0;
    combined_flag=1;
    for(i=0;i<10;i++)
      {
        if(i==0){for(j=9;j<12;j++){Floor[i][j].status=1;}}
        else if(i==1){for(j=9;j<12;j++){Floor[i][j].status=1;}} 
        else if(i==2){for(j=9;j<12;j++){Floor[i][j].status=1;}}
        else if(i==3){for(j=0;j<6;j++){Floor[i][j].status=1;}for(j=9;j<15;j++){Floor[i][j].status=1;}}
        else if(i==4){for(j=0;j<6;j++){Floor[i][j].status=1;}for(j=9;j<15;j++){Floor[i][j].status=1;}}
        else if(i==5){for(j=0;j<6;j++){Floor[i][j].status=1;}for(j=9;j<15;j++){Floor[i][j].status=1;}}
        else if(i==6){for(j=9;j<12;j++){Floor[i][j].status=1;}}
        else if(i==7){for(j=9;j<12;j++){Floor[i][j].status=1;}}              
        else if(i==8){for(j=9;j<12;j++){Floor[i][j].status=1;}}
      }
      Floor[4][13].status=4;
      Floor[4][4].status=5;
      Floor[4][12].status=7;
      Floor[4][11].status=7;
      level_x_trans=-1;
      level_y_trans=0;
      level_z_trans=-1;
      cube_x_trans=-1;
      cube_y_trans=0;
      cube_z_trans=1;    

  }
  



  for(i=0;i<10;i++)
  {
    for(j=0;j<15;j++)
    {
      if(Floor[i][j].status==2)
      {
      for(k=12;k<18;k++)
      {
        if(k%3==0)
        {
          Floor[i][j].floor_color_buffer[3*k]=1;
          Floor[i][j].floor_color_buffer[3*k+1]=255/255.0;
        Floor[i][j].floor_color_buffer[3*k+2]=255/255.0;
        }
        if(k%15==0)
         {
          Floor[i][j].floor_color_buffer[3*k]=204/255.0;
        Floor[i][j].floor_color_buffer[3*k+1]=255/255.0;
        Floor[i][j].floor_color_buffer[3*k+2]=255/255.0;
          }
          /*
        Floor[i][j].floor_color_buffer[3*k]=204/255.0;
        Floor[i][j].floor_color_buffer[3*k+1]=255/255.0;
        Floor[i][j].floor_color_buffer[3*k+2]=255/255.0;
         */
      }
      }
      if(Floor[i][j].status==3)
      {
      for(k=12;k<18;k++)
      {
        Floor[i][j].floor_color_buffer[3*k]=0;
        Floor[i][j].floor_color_buffer[3*k+1]=0;
        Floor[i][j].floor_color_buffer[3*k+2]=1;
         
      }
      }
      if(Floor[i][j].status==5)
      {
      for(k=12;k<18;k++)
      {
        Floor[i][j].floor_color_buffer[3*k]=1;
        Floor[i][j].floor_color_buffer[3*k+1]=0;
        Floor[i][j].floor_color_buffer[3*k+2]=1;
         
      }
    } 

      if(Floor[i][j].status==6 )
      {
      for(k=12;k<18;k++)
      {
        Floor[i][j].floor_color_buffer[3*k]=0;
        Floor[i][j].floor_color_buffer[3*k+1]=1;
        Floor[i][j].floor_color_buffer[3*k+2]=0;
      }
      }
      if(Floor[i][j].status==7 )
      {
      for(k=12;k<18;k++)
      {
        Floor[i][j].floor_color_buffer[3*k]=102/255.0;
        Floor[i][j].floor_color_buffer[3*k+1]=102/255.0;
        Floor[i][j].floor_color_buffer[3*k+2]=0/255.0;
      }
      }
  }
  }


  for(i=0;i<10;i++)
  {
    for(j=0;j<15;j++)
    {
      Floor[i][j].Object=create3DObject(GL_TRIANGLES, 36, Floor[i][j].floor_vertex_buffer, Floor[i][j].floor_color_buffer, GL_FILL);
    }
  }

}

/*CUBE CREATION*/
void create_cube1(float l,float b,float h)
{
  l=l/2;
  h=h/2;
  b=b/2;
  GLfloat a[]={

      /* Rectangle 1 */

      -l, -b, -h,
      -l, -b, h,
      l, -b, h,
      l, -b, h,
      l, -b, -h,
      -l, -b, -h,

      /* Rectangle 2 */

      l, b, -h,
      l, b, h,
      l, -b, h,
      l, -b, h,
      l, -b, -h,
      l, b, -h,

      /* Rectangle 3 */

      -l, b, -h,
      -l, b, h,
      l, b, h,
      l, b, h,
      l, b, -h,
      -l, b, -h,

      /* Rectangle 4 */

      -l, b, -h,
      -l, b, h,
      -l, -b, h,
      -l, -b, h,
      -l, -b, -h,
      -l, b, -h,

      /* Rectangle 5 */

      -l, -b, -h,
      -l, b, -h,
      l, b, -h,
      l, b, -h,
      l, -b, -h,
      -l, -b, -h,

      /* Rectangle 6 */
      -l, -b, h,
      -l, b, h,
      l, b, h,
      l, b, h,
      l, -b, h,
      -l, -b, h,

  };

  GLfloat color_buffer_data [108];
  int k;
  for(k=0;k<36;k++)
      {
          color_buffer_data[3*k]=218/255;
          color_buffer_data[3*k+1]=165/255;
          color_buffer_data[3*k+2]=32/255;

          if(k%3==0 || k%5==0 || k%7==0 || k%8==0 ||  k%13==0)
        {//Floor[i][j].floor_color_buffer[3*k]=1;
          color_buffer_data[3*k]=255/255.0;
          color_buffer_data[3*k+1]=223/255.0;
          color_buffer_data[3*k+2]=0/255.0;
        }
      }
  cube1.Object=create3DObject(GL_TRIANGLES,  6*6,  a,  color_buffer_data, GL_FILL );
  cube1.x=-1-0.05;
  cube1.y=0.5;
  cube1.z=-1+0.1;
  
  cube1.flag=1;

  cube2.Object=create3DObject(GL_TRIANGLES,  6*6,  a,  color_buffer_data, GL_FILL );
  cube2.x=-1-0.05;
  cube2.y=1.0;
  cube2.z=-1+0.1;
  
  cube2.flag=1;
  /*if(cur_level==1)
  {
    cube1.x=-1-0.05;
    cube1.y=0.5;
    cube1.z=-1+0.1;
    cube2.x=-1-0.05;
    cube2.y=1.0;
    cube2.z=-1+0.1;
  }
  if(cur_level==2)
  { 
    cube1.x=-1-0.05;
    cube1.y=0.5;
    cube1.z=-1+0.1;
    cube2.x=-1-0.05;
    cube2.y=1.0;
    cube2.z=-1+0.1;
  }*/

}

void set_characters(char arr,Things char_seg[])
{
  for(int i=0;i<12;i++)
    char_seg[i].status=0;
  
    char cur=arr;
    //left1
    if(cur=='P' || cur=='O' || cur=='N' || cur=='S' || cur=='Y' || cur=='U'|| cur=='W' || cur=='L'|| cur=='E' || cur=='1' || cur=='0'|| cur=='4'|| cur=='5'|| cur=='6'|| cur=='8'|| cur=='9' || cur=='A' || cur=='G')
    {
      char_seg[0].status=1;

    }
    //left2
    if(cur=='P' || cur=='O' || cur=='N'  || cur=='U'|| cur=='W'|| cur=='L'|| cur=='E'|| cur=='1'|| cur=='0'|| cur=='2'||  cur=='6'|| cur=='8' || cur=='A' || cur=='G')
    {
      char_seg[1].status=1;
    }
    //right1
    if(cur=='U' ||cur=='P' || cur=='O' || cur=='N'  || cur=='Y'|| cur=='W'|| cur=='0'|| cur=='2'|| cur=='3'|| cur=='4'|| cur=='8'|| cur=='7'|| cur=='9' || cur=='A' )
    {
     char_seg[2].status=1; 
    }
    //right2
    if(cur=='O' ||  cur=='N' || cur=='S' || cur=='Y' || cur=='U'|| cur=='W'|| cur=='0'|| cur=='3'|| cur=='5'|| cur=='6'|| cur=='4'|| cur=='8'|| cur=='7'|| cur=='9' || cur=='A' || cur=='G')
    {
      char_seg[3].status=1;
    }
    //top
    if(cur=='P' || cur=='O' || cur=='I' || cur=='N' || cur=='T' || cur=='S'|| cur=='E'|| cur=='0'|| cur=='2'|| cur=='3'|| cur=='5'|| cur=='6'|| cur=='8' || cur=='7'|| cur=='9' || cur=='A' || cur=='G')
    {
      char_seg[4].status=1;
    }
    //middle
    if(cur=='P' || cur=='S'  || cur=='Y' || cur=='E'|| cur=='2'|| cur=='3'|| cur=='4'|| cur=='5'|| cur=='6'|| cur=='8'|| cur=='9' || cur=='-' || cur=='A' || cur=='G')
    {
      char_seg[5].status=1;
    }
    //bottom
    if(cur=='O' || cur=='I' || cur=='S'  || cur=='Y' || cur=='U'|| cur=='L' || cur=='E'|| cur=='0'|| cur=='2'|| cur=='3'|| cur=='5'|| cur=='6'|| cur=='8'|| cur=='9'||  cur=='G' || cur=='W')
    {
      char_seg[6].status=1;
    }
    
    //middle1
    if(cur=='I' || cur=='T' )
    {
      char_seg[7].status=1;
    }
    //middle2
    if(cur=='I' || cur=='T' || cur=='W')
    {
      char_seg[8].status=1;
    }

  }



float rectangle_rotation = 0;
float triangle_rotation = 0;
float min(float a,float b)
{
  if(a>b)
  return b;
else 
  return a;
}
float max(float a,float b)
{
  if(a<b)
  return b;
else 
  return a;
}


/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 7, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  if(v==1)//TOWER VIEW
  {
    Matrices.projection = glm::ortho((float)(-6.0f/zoom_camera+x_change), (float)(6.0f/zoom_camera+x_change), (float)(-6.0f/zoom_camera+y_change), (float)(6.0f/zoom_camera+y_change), 0.1f, 500.0f);
    Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D

  }
  if(v==2)//TOP VIEW
   {
    Matrices.projection = glm::ortho((float)(-6.0f/zoom_camera+x_change), (float)(6.0f/zoom_camera+x_change), (float)(-6.0f/zoom_camera+y_change), (float)(6.0f/zoom_camera+y_change), 0.1f, 500.0f);
    Matrices.view = glm::lookAt(glm::vec3(-2,10,2), glm::vec3(0,0,0), glm::vec3(0,0,-1)); 
    }
    if(v==3){//HELICOPTER VIEW
      if(right_clicked==1){
      camera_rotation_angle+=(-m1x+mouse_x)*10;
      m1x=mouse_x;
      tower_eye_x=5*cos(camera_rotation_angle*M_PI/180.0f);
      tower_eye_y+=(-m1y+mouse_y)/10;
      if(tower_eye_y>9)
        tower_eye_y=9;
      if(tower_eye_y<-4)
        tower_eye_y=-4;
      tower_eye_z=5*sin(camera_rotation_angle*M_PI/180.0f);
        }
    Matrices.projection = glm::ortho((float)(-6.0f/zoom_camera+x_change), (float)(6.0f/zoom_camera+x_change), (float)(-6.0f/zoom_camera+y_change), (float)(6.0f/zoom_camera+y_change), 0.1f, 500.0f);
    Matrices.view = glm::lookAt(glm::vec3(tower_eye_x,tower_eye_y,tower_eye_z), glm::vec3(0,0,0), glm::vec3(0,1,0));
 }

 if(v==4)//FOLLOW VIEW
  {
    if(right_clicked==1){
      camera_rotation_angle+=(-m1x+mouse_x)*10;
      m1x=mouse_x;
      tower_eye_x=3*cos(camera_rotation_angle*M_PI/180.0f);
      tower_eye_z=3*sin(camera_rotation_angle*M_PI/180.0f);
        }
    Matrices.projection = glm::perspective (90.0f, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);
    //Matrices.projection = glm::ortho((float)(-6.0f/zoom_camera+x_change), (float)(6.0f/zoom_camera+x_change), (float)(-6.0f/zoom_camera+y_change), (float)(6.0f/zoom_camera+y_change), 0.1f, 500.0f);
    Matrices.view = glm::lookAt(glm::vec3(min(cube1.x+cube_x_trans-0.3,cube2.x+cube_x_trans-0.3),max(cube1.y+cube_y_trans,cube2.y+cube_y_trans)+1.2,min(cube1.z+cube_z_trans-0.3,cube2.z+cube_z_trans-0.3)), glm::vec3(max(cube1.x+cube_x_trans+1,cube2.x+cube_x_trans+1)+tower_eye_x,0,max(cube1.z+cube_z_trans+1,cube2.z+cube_z_trans+1)+tower_eye_z), glm::vec3(0,1,0));


  }
  if(v==5)//BLOCK VIEW
  {
    if(right_clicked==1){
      camera_rotation_angle+=(-m1x+mouse_x)*10;
      m1x=mouse_x;
      tower_eye_x=5*cos(camera_rotation_angle*M_PI/180.0f);
      tower_eye_z=5*sin(camera_rotation_angle*M_PI/180.0f);
        }
    Matrices.projection = glm::perspective (90.0f, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);
    //Matrices.projection = glm::ortho((float)(-6.0f/zoom_camera+x_change), (float)(6.0f/zoom_camera+x_change), (float)(-6.0f/zoom_camera+y_change), (float)(6.0f/zoom_camera+y_change), 0.1f, 500.0f);
    Matrices.view = glm::lookAt(glm::vec3(max(cube1.x+cube_x_trans+0.3,cube2.x+cube_x_trans+0.3),0.75,max(cube1.z+cube_z_trans+0.3,cube2.z+cube_z_trans+0.3)), glm::vec3(min(cube1.x+cube_x_trans+0.5,cube2.x+cube_x_trans+0.5)+tower_eye_x,0.5,min(cube1.z+cube_z_trans+0.5,cube2.z+cube_z_trans+0.5)+tower_eye_z), glm::vec3(0,1,0));

  }
  //  Don't change unless you are sure!!
  //Matrices.view = glm::lookAt(glm::vec3(2,2,3), glm::vec3(0,0,1), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
   Matrices.view1 = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 VP1 = Matrices.projection * Matrices.view1;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  /*glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform;
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);*/

COLOR winbackground = {212/255.0,175/255.0,55/255.0};
  COLOR losebackground = {255/255.0,77/255.0,77/255.0};
  if(cur_score<=win_score && game_over_var){
        //game_over_var=1;
        //endLabel_x=-1.5;
        /*createRectangle("endgame",10000,winbackground,winbackground,winbackground,winbackground,0,0,2,3,"background");
        */endLabel="YOU WIN";
    }

    else if(cur_score>win_score)
    {
        game_over_var=1;
        /*createRectangle("endgame",10000,losebackground,losebackground,losebackground,losebackground,0,0,2,3,"background");
        */endLabel="YOU LOSE";
    }

    //mouse_controls for panning
    glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);
    if(right_mouse_clicked==1){
        x_change+=new_mouse_pos_x-mouse_pos_x;
        y_change-=new_mouse_pos_y-mouse_pos_y;
        check_pan();
    }
    Matrices.projection = glm::ortho((float)(-6.0f/zoom_camera+x_change), (float)(6.0f/zoom_camera+x_change), (float)(-6.0f/zoom_camera+y_change), (float)(6.0f/zoom_camera+y_change), 0.1f, 500.0f);
    glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);

if(flag2!=flag3 && flag2%2==0){
    if(m2_z<m1_z && m1_z-m2_z>=0.5 && 
      ((m2_x>=cube1.x+cube_x_trans-0.25 && m2_x<=cube1.x+cube_x_trans+0.25) || 
        (m2_x>=cube2.x+cube_x_trans-0.25 && m2_x<=cube2.x+cube_x_trans+0.25)))
      move(4);
    if(m2_z>m1_z && m1_z-m2_z<=0.5 && 
      ((m2_x>=cube1.x+cube_x_trans-0.25 && m2_x<=cube1.x+cube_x_trans+0.25) || 
        (m2_x>=cube2.x+cube_x_trans-0.25 && m2_x<=cube2.x+cube_x_trans+0.25)))
      move(3);
    if(m2_x<m1_x && m1_x-m2_x>=0.5 && 
      ((m2_z>=cube1.y+cube_y_trans-0.5 && m2_z<=cube1.y+cube_y_trans+0.5) || (m2_z>=cube2.y+cube_y_trans-0.5 && m2_z<=cube2.y+cube_y_trans+0.5)))
      move(2);
    if(m2_x>m1_x && m1_x-m2_x<=0.5 && ((m2_z>=cube1.y+cube_y_trans-0.5 && m2_z<=cube1.y+cube_y_trans+0.5) || (m2_z>=cube2.y+cube_y_trans-0.5 && m2_z<=cube2.y+cube_y_trans+0.5)))
      move(1);
    flag3=flag2;
  }



/*FLOOR CREATION*/

if(!game_over_var)
{
floor_falling_anim+=0.2;
   if(floor_falling_anim>Floor[0][0].y+level_y_trans)
   {
    floor_falling_anim=Floor[0][0].y+level_y_trans;
   }
for(int i=0;i<10;i++)
{

for(int j=0;j<15;j++)
{
  if(Floor[i][j].status !=0 && Floor[i][j].status !=4 )
  {
  Matrices.model = glm::mat4(1.0f);
  /*floor_falling_anim+=(i+j)/1.5;
   if(floor_falling_anim>Floor[0][0].y+level_y_trans)
   {
    floor_falling_anim=0;
   }*/
  /*Floor[i][j].y+=((i+j)/3)+0.4;
  if(Floor[i][j].y>0)
  {
    Floor[i][j].y=0;
  }*/
  glm::mat4 translate_floor = glm::translate (glm::vec3(Floor[i][j].x+level_x_trans+floor_falling_anim,floor_falling_anim+Floor[i][j].y+level_y_trans,Floor[i][j].z+level_z_trans));        //zglTranslatef
  //glm::mat4 rotateRectangle1 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0 )); // rotate about vector (-1,1,1)
  glm::mat4 scale_floor = glm::scale(glm::vec3(1,1,1));
  Matrices.model *= (translate_floor*scale_floor);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(Floor[i][j].Object);

}}
}

/*CUBE CREATION*/
  Matrices.model = glm::mat4(1.0f);
  /*if(cube1.flag==2)
  {
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 trans_cube1_right=glm::translate (glm::vec3(-cube1.x-0.25,-cube1.y+0.5-0.5,-cube1.z));
    glm::mat4 rotate_cube1_right = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,0,cube1.z)); // rotate about vector (-1,1,1)  
    glm::mat4 trans_cube1_right_rev=glm::translate (glm::vec3(cube1.x+0.25,cube1.y-0.5+1,cube1.z));
    Matrices.model *= (trans_cube1_right_rev*rotate_cube1_right*trans_cube1_right);
    MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
   draw3DObject(cube1.Object); 
  }
  else
  {*/

   disable_controls=1; 
  /*LOWER CUBE CREATION*/
   cube_falling_anim-=0.2;
   if(cube_falling_anim<0)
   {
    disable_controls=0;
    cube_falling_anim=0;
   }


  Matrices.model = glm::mat4(1.0f); 
  glm::mat4 translate_cube1 = glm::translate (glm::vec3(cube1.x+cube_x_trans,cube1.y+cube_falling_anim+cube_y_trans,cube1.z+cube_z_trans));        //zglTranslatef
  //glm::mat4 rotateRectangle1 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0 )); // rotate about vector (-1,1,1)
  //glm::mat4 scale_floor = glm::scale(glm::vec3(1,0.5,1));
  Matrices.model *= (translate_cube1);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(cube1.Object);

  /*UPPER CUBE CREATION*/
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translate_cube2 = glm::translate (glm::vec3(cube2.x+cube_x_trans,cube2.y+cube_falling_anim+cube_y_trans,cube2.z+cube_z_trans));        //zglTranslatef
  Matrices.model *= (translate_cube2);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(cube2.Object);
}

  /*MAPPING THE CENTRE COORDINATES TO MATRIX*/
  int l1,r1,l2,r2;
  r1=(int)floor(((cube1.x+1.5+0.05-level_x_trans+cube_x_trans)*2)+0.0001);
  l1=(int)floor(((cube1.z+1.5-0.1-level_z_trans+cube_z_trans)*2)+0.0001);
  r2=(int)floor(((cube2.x+1.5+0.05-level_x_trans+cube_x_trans)*2)+0.0001);
  l2=(int)floor(((cube2.z+1.5-0.1-level_z_trans+cube_z_trans)*2)+0.0001);
  //printf("%lf\n",((cube1.z+1.5-0.1-level_z_trans)*2));
  //printf("%d %d %d %d\n",l1,r1,l2,r2);
  //printf("%lf %lf %lf %lf %lf %lf\n",cube1.x,cube1.y,cube1.z,cube2.x,cube2.y,cube2.z);
  /*double l11,l21,r11,r21;
  r11=(double)floor(((cube1.x+1.5+0.05-level_x_trans)*2));
  l11=(double)floor(((cube1.z+1.5-0.1-level_z_trans)*2));
  r21=(double)floor(((cube2.x+1.5+0.05-level_x_trans)*2));
  l21=(double)floor(((cube2.z+1.5-0.1-level_z_trans)*2));
  //printf("%lf %lf %lf %lf\n",l11,r11,l21,r21);
  */
  /*CHECK FALLING OF BLOCKS*/
  if(l1<0 || l2<0 || r1<0 || r2<0 || Floor[l1][r1].status==0 || Floor[l2][r2].status==0 || l1>=10 || l2>=10 || r1>=15 || r2>=15)
  {
    //prev_score=cur_score;
    disable_controls=1;
    double cur_time=glfwGetTime();
    if(cur_time-utime>0.05)
    {
      utime=glfwGetTime();
      //cube1.y-=0.2;
      //cube2.y-=0.2;
      //disable_controls=1;
      if(teleport==0)
      {
        cube1.y-=0.2;
      cube2.y-=0.2;
      }
      else if(teleport==1)
      {
        //cube1.y-=1;
        cube2.y-=0.2;
      }
      else if(teleport==2)
      {
        cube1.y-=0.2;
      //cube2.y-=1;
      }
    }
    if(cube1.y<-3 || cube2.y<-3)
    {
      disable_controls=0;
      create_floor();
      create_cube1(0.5,0.5,0.5);
    }

}

if(l1>=0 && l2>=0 && r1>=0 && r2>=0 && Floor[l1][r1].status==2 && Floor[l2][r2].status==2 && l1==l2 && r1==r2)
  {
    //prev_score=cur_score;
    disable_controls=1;
    double cur_time=glfwGetTime();
    if(cur_time-utime>0.05)
    {
      utime=glfwGetTime();
      cube1.y-=0.2;
      cube2.y-=0.2;
      Floor[l1][r1].y-=0.4;
      //disable_controls=1;
    }
    if(cube1.y<-3)
    {
      disable_controls=0;
      create_floor();
      create_cube1(0.5,0.5,0.5);
    }
  }

  if(l1>=0 && l2>=0 && r1>=0 && r2>=0 && Floor[l1][r1].status==4 && Floor[l2][r2].status==4 )
  {
    prev_score=cur_score;
    double cur_time=glfwGetTime();
    if(cur_time-utime>0.05)
    {
      utime=glfwGetTime();
      cube1.y-=1;
      cube2.y-=1;
      disable_controls=1;
    }
    if(cube1.y<-3)
    {
      cur_level++;
      disable_controls=0;
      create_floor();
      create_cube1(0.5,0.5,0.5);
    }
    if(cur_level==6)
      game_over();
  }




if(cur_level==2)
{
  if((l1>=0 && l2>=0 && r1>=0 && r2>=0) && (Floor[l1][r1].status==3 || Floor[l2][r2].status==3) && !(bridge1))
  {
    //thread(play_audio,"./bridge.mp3").detach();    
    //system("canberra-gtk-play -f ./bridge.wav");
    bridge1_1=1;
    Floor[4][4].status=1;
    Floor[4][5].status=1;
  }
  else if((l1>=0 && l2>=0 && r1>=0 && r2>=0) && (Floor[l1][r1].status==3 || Floor[l2][r2].status==3) && (bridge1))
  {
    //thread(play_audio,"./bridge.mp3").detach();
    bridge1_1=0;
    Floor[4][4].status=0;
    Floor[4][5].status=0;
  }
  else if(bridge1_1==1)
  {
    bridge1=1;
  }
  else if(bridge1_1==0)
  {
   bridge1=0; 
  }

  if((l1>=0 && l2>=0 && r1>=0 && r2>=0) && (Floor[l1][r1].status==6 && Floor[l2][r2].status==6) && !(bridge2))
  {
    //thread(play_audio,"./bridge.mp3").detach();
    bridge2_1=1;
    Floor[4][10].status=1;
    Floor[4][11].status=1;
  }
  else if((l1>=0 && l2>=0 && r1>=0 && r2>=0) && (Floor[l1][r1].status==6 && Floor[l2][r2].status==6) && (bridge2))
  {
    //thread(play_audio,"./bridge.mp3").detach();
    bridge2_1=0;
    Floor[4][10].status=0;
    Floor[4][11].status=0;
  }
  else if(bridge2_1==1)
  {
    bridge2=1;
  }
  else if(bridge2_1==0)
  {
   bridge2=0; 
  }

   Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate_circle1 = glm::translate (glm::vec3(circle1[0].x+floor_falling_anim, circle1[0].y+floor_falling_anim, circle1[0].z));        // glTranslatef
  glm::mat4 rotate_circle1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate_circle1 );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(circle1[0].Object);


   Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate_circle2 = glm::translate (glm::vec3(circle1[1].x+floor_falling_anim, circle1[1].y+floor_falling_anim, circle1[1].z));        // glTranslatef
  glm::mat4 rotate_circle2 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate_circle2 );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(circle1[1].Object);


   Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate_circle3 = glm::translate (glm::vec3(circle1[2].x+floor_falling_anim, circle1[2].y+floor_falling_anim, circle1[2].z));        // glTranslatef
  glm::mat4 rotate_circle3 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate_circle3 );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(circle1[2].Object);
  
}
if(cur_level==4)
{
  if((l1>=0 && l2>=0 && r1>=0 && r2>=0) && (Floor[l1][r1].status==6 && Floor[l2][r2].status==6) && !(bridge1))
  {
    bridge1_1=1;
    Floor[5][9].status=1;
    Floor[6][9].status=1;
  }
  else if((l1>=0 && l2>=0 && r1>=0 && r2>=0) && (Floor[l1][r1].status==6 && Floor[l2][r2].status==6) && (bridge1))
  {
    bridge1_1=0;
    Floor[5][9].status=0;
    Floor[6][9].status=0;
  }
  else if(bridge1_1==1)
  {
    bridge1=1;
  }
  else if(bridge1_1==0)
  {
   bridge1=0; 
  }
}
if(cur_level==5)
{
  if((l1>=0 && l2>=0 && r1>=0 && r2>=0) && (Floor[l1][r1].status==5 && Floor[l2][r2].status==5))
  {
    cube1.x+=3;
    cube1.z-=1.5;
    cube2.x+=3;
    cube2.z+=1.5;
    cube1.y=cube2.y;
    teleport=1;
    combined_flag=0;
  }
  if(r1==11 && l1==4 && l2==4 && r2==12)
  {
    teleport=0;
    combined_flag=1;
  }
  else if(l2==4 && r2== 12 && combined_flag==0)
  {
    teleport=2;
  }

}

//SCORE LABEL
base_x=scoreLabel_x;
  base_y=scoreLabel_y;
  for(int z=0;z<scoreLabel.length();z++)
  {

  set_characters(scoreLabel[z],scoreLabelObjects);
  for(int rem=0;rem<12;rem++)
  {
    if(scoreLabelObjects[rem].status==1)
    {
    int cur=rem;//first object
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_score_label_matrix = glm::translate (glm::vec3(base_x+scoreLabelObjects[cur].x, base_y+scoreLabelObjects[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_score_label_matrix = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_score_label_matrix * rotate_score_label_matrix);
    MVP = VP1 * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(scoreLabelObjects[cur].Object);
    }
  }
  base_x+=0.4;
}

//score value label
  base_x=score_value_x;
  base_y=score_value_y;
  char score_value[100];
  int temp=cur_score;
  if(cur_score<0)
    {
      //score_value[0]='-';
      temp=-1*cur_score;
    }
  
for(int i=0;i<100;i++)
  score_value[i]='.';
  score_value[3]='0';
  int x=3;
  while(temp)
  {
    score_value[x]=(temp%10)+'0';
    temp/=10;
    x--;
  }
  if(cur_score<0)
    score_value[x]='-';
  
  for(int z=0;z<=3;z++)
  {
    if(score_value[z]=='.')
      continue;
    set_characters(score_value[z],score_valueObjects);
  for(int rem=0;rem<12;rem++)
  {
    if(score_valueObjects[rem].status==1)
    {
    int cur=rem;//first object
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_score_value_matrix = glm::translate (glm::vec3(base_x+score_valueObjects[cur].x, base_y+score_valueObjects[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_score_value_matrix = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_score_value_matrix * rotate_score_value_matrix);
    MVP = VP1 * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(score_valueObjects[cur].Object);
    }
  }
  base_x+=0.4;
}
if(cur_level<=5)
{
//STAGE LABEL
base_x=levelLabel_x;
  base_y=levelLabel_y;
  for(int z=0;z<levelLabel.length();z++)
  {

  set_characters(levelLabel[z],levelLabelObjects);
  for(int rem=0;rem<12;rem++)
  {
    if(levelLabelObjects[rem].status==1)
    {
    int cur=rem;//first object
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_level_label_matrix = glm::translate (glm::vec3(base_x+levelLabelObjects[cur].x, base_y+levelLabelObjects[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_level_label_matrix = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_level_label_matrix * rotate_level_label_matrix);
    MVP = VP1 * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(levelLabelObjects[cur].Object);
    }
  }
  base_x+=0.4;
}

// "LEVEL" label
base_x=level_value_x;
base_y=level_value_y;
char level_value[2];
for(int i=0;i<2;i++)
  level_value[i]='0';
  //level_value[1]='0';
   x=1;
  temp=cur_level;
  while(temp)
  {
    level_value[x]=(temp%10)+'0';
    temp/=10;
    x--;
  }
for(int z=0;z<=1;z++)
  {
    set_characters(level_value[z],level_valueObjects);
  for(int rem=0;rem<12;rem++)
  {
    if(level_valueObjects[rem].status==1)
    {
    int cur=rem;//first object
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_level_value_matrix = glm::translate (glm::vec3(base_x+level_valueObjects[cur].x, base_y+level_valueObjects[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_level_value_matrix = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_level_value_matrix * rotate_level_value_matrix);
    MVP = VP1 * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(level_valueObjects[cur].Object);
    }
  }
  base_x+=0.4;
}
}
//"GAME OVER" label
  base_x=endLabel_x;
  base_y=endLabel_y;
  for(int z=0;z<endLabel.length();z++)
  {

  set_characters(endLabel[z],endLabelObjects);
  for(int rem=0;rem<12;rem++)
  {
    if(endLabelObjects[rem].status==1)
    {
    int cur=rem;//first object
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_end_label_matrix = glm::translate (glm::vec3(base_x+endLabelObjects[cur].x, base_y+endLabelObjects[cur].y, 0.0f)); // glTranslatef
    glm::mat4 rotate_end_label_matrix = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translate_end_label_matrix * rotate_end_label_matrix);
    MVP = VP1 * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(endLabelObjects[cur].Object);
    }
  }
  base_x+=1;
}







  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, mousescroll);
    glfwSetCursorPosCallback(window, cursor_position);


    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
  COLOR white = {255/255.0,255/255.0,255/255.0};
  COLOR cream = {160/255.0,160/255.0,160/255.0};
  COLOR score = {117/255.0,78/255.0,40/255.0};
  COLOR darkgreen = {51/255.0,102/255.0,0/255.0};
  COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};


    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	//createRectangle ();
  create_floor();
  create_cube1(0.5,0.5,0.5);
  createCircle(0,white,-0.5-1-0.05,0.25,-0.5-1+0.1,0.125,1,0);
  createCircle(1,white,-0.5-1-0.05+3,0.25,-0.5-1+0.1-0.5,0.175,1,0);
  createCircle(2,cream,-0.5-1-0.05+3,0.25,-0.5-1+0.1-0.5,0.09125,1,0);
  
  //ALL LABELS
  int t;
    for(t=0;t<=4;t++)
    {
        string layer;
        float wid,heig,offset;
        wid=0.35;heig=0.1;offset=0.25;
        COLOR color = score;
      if(t==0)
        {
          layer="scorelabel";
          color=white;
        }
      if(t==1)
      {
        layer="endlabel";
        wid=0.7;heig=0.2;offset=0.5;color=red;
      }
      if(t==2)
      {
        layer="score_value";
      }
      if(t==3)
      {
        layer="level_value";
        color=white;

      }
      if(t==4)
      {
        layer="levellabel";
      }

        createRectangle("top",4,color,color,color,color,0,offset,heig,wid,layer);
        createRectangle("bottom",6,color,color,color,color,0,-offset,heig,wid,layer);
        createRectangle("middle",5,color,color,color,color,0,0,heig,wid,layer);
        createRectangle("left1",0,color,color,color,color,-offset/2,offset/2,wid,heig,layer);
        createRectangle("left2",1,color,color,color,color,-offset/2,-offset/2,wid,heig,layer);
        createRectangle("right1",2,color,color,color,color,offset/2,offset/2,wid,heig,layer);
        createRectangle("right2",3,color,color,color,color,offset/2,-offset/2,wid,heig,layer);
        createRectangle("middle1",7,color,color,color,color,0,offset/2,wid,heig,layer);
        createRectangle("middle2",8,color,color,color,color,0,-offset/2,wid,heig,layer);
     }



	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (113/255.0,185/255.0,209/255.0,0.4f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
time_t old_time;
int main (int argc, char** argv)
{
	int width = 1200;
	int height = 800;

    GLFWwindow* window = initGLFW(width, height);
    old_time=time(NULL);
    thread(play_audio,"./Audio1.mp3").detach();


	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        if((time(NULL)-old_time>=120) && (!game_over_var))
        {
        old_time=time(NULL);
        thread(play_audio,"./Audio1.mp3").detach();
        }
        // OpenGL Draw commands
        draw(window);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
