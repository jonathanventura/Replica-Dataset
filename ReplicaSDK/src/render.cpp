// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#include <EGL.h>
#include <PTexLib.h>
#include <pangolin/image/image_convert.h>

#include "GLCheck.h"
#include "MirrorRenderer.h"

//added for safety
#include <pangolin/display/opengl_render_state.h>

// added for convenience
#ifdef HAVE_GLES
    typedef float GLPrecision;
#else
    typedef double GLPrecision;
#endif
// handle when file format decided
// class InvalidFormatException: public exception
// {
//   virtual const char* what() const throw()
//   {
//     return "The camera poses file is formatted invalidly";
//   }
// } InvalidFormatEx;


class CameraPose{

  public:
    GLPrecision ex, ey, ez, lx, ly, lz, ux, uy, uz;
    std::string filename;
    CameraPose(
        GLPrecision e_x, 
        GLPrecision e_y, 
        GLPrecision e_z, 
        GLPrecision l_x, 
        GLPrecision l_y, 
        GLPrecision l_z, 
        GLPrecision u_x, 
        GLPrecision u_y, 
        GLPrecision u_z,
        std::string file_name);
    
};

CameraPose::CameraPose(
        GLPrecision e_x, 
        GLPrecision e_y, 
        GLPrecision e_z, 
        GLPrecision l_x, 
        GLPrecision l_y, 
        GLPrecision l_z, 
        GLPrecision u_x, 
        GLPrecision u_y, 
        GLPrecision u_z,
        std::string file_name)
{
  ex = e_x;
  ey = e_y;
  ez = e_z;
  lx = l_x;
  ly = l_y;
  lz = l_z;
  ux = u_x;
  uy = u_y;
  uz = u_z;
  filename = file_name;
} 

float rand_float(){
  return (((float)rand()) / RAND_MAX)*2) -1;
}
  
void generate_randomPoses(std::vector <CameraPose> &poses,
                          int num_poses, std::string filename
                          int ex, int ey, int ez) {

  for(int i=7; i<(7+num_poses); i++){
      poses.push_back(
        CameraPose(ex + rand_float(), ey + rand_float(), ez + rand_float(), 
                   ex + rand_float(), ey + rand_float(), ez + rand_float(),, 
                   0, 0, 1, std::format("{}_{:02}", filename,i))
      );
  }
    

}
//cube_##_##_depth
//cube_##_##_frame
std::vector <CameraPose> createPoses(std::string textfile, float dist)
{
  int cube_num, rand_pos;
  std::string line, filename;
  GLPrecision ex, ey, ez, lx, ly, lz, ux, uy, uz;
  std::vector <CameraPose> poses;
  std::ifstream file(textfile);
  
  //constant
  rand_pos = 10
  //modify later
  ux = 0;
  uy = 0;
  uz = 1;

  cube_num = 0

  while (getline(file, line))
  {
    //check_valid_line(line);
    std::istringstream iss (line);
    iss >> ex >> ey >> ez;
    lx = ex;
    ly = ey;
    lz = ez;

    filename = std::format("cube_{:02}", cube_num);

    // assumes poses per line, no error handling atm
    
    poses.push_back(CameraPose(ex, ey, ez, lx + dist, ly, lz, ux, uy, uz, std::format("{}_{}", filename,"01"));
    poses.push_back(CameraPose(ex, ey, ez, lx, ly - dist, lz, ux, uy, uz, std::format("{}_{}", filename,"02"));

    poses.push_back(CameraPose(ex, ey, ez, lx - dist, ly, lz, ux, uy, uz, std::format("{}_{}", filename,"03"));
    poses.push_back(CameraPose(ex, ey, ez, lx, ly + dist, lz, ux, uy, uz, std::format("{}_{}", filename,"04"));

    poses.push_back(CameraPose(ex, ey, ez, lx, ly, lz + dist, ux, uz, uy, std::format("{}_{}", filename,"05"));
    poses.push_back(CameraPose(ex, ey, ez, lx, ly, lz - dist, ux, -uz, uy, std::format("{}_{}", filename,"06"));

    generate_randomPoses(poses, rand_pos, filename, ex, ey, ez);

    ++cube_num;
  }
  return poses;
}

int main(int argc, char* argv[]) {

  srand(time(NULL));

  // modified so command line handles poses files and so it's more convenient
  ASSERT(argc == 4 || argc == 5, "Usage: ./ReplicaRenderer posesFile mesh.ply /path/to/atlases [mirrorFile]");
  
  const std::string textfile(argv[1]);
  const std::string meshFile(argv[2]);
  const std::string atlasFolder(argv[3]);

  // file exists is general purpose
  ASSERT(pangolin::FileExists(textfile));
  ASSERT(pangolin::FileExists(meshFile));
  ASSERT(pangolin::FileExists(atlasFolder));

  std::string surfaceFile;
  if (argc == 5) {
    surfaceFile = std::string(argv[4]);
    ASSERT(pangolin::FileExists(surfaceFile));
  }
  // end modified
  const int width = 1280;
  const int height = 1280;
  bool renderDepth = true;
  float depthScale = 65535.0f * 0.1f;
  const float renderDistance = 0.5;

  // Setup EGL
  EGLCtx egl;

  egl.PrintInformation();
  
  if(!checkGLVersion()) {
    return 1;
  }

  //Don't draw backfaces
  const GLenum frontFace = GL_CCW;
  glFrontFace(frontFace);

  // Setup a framebuffer
  pangolin::GlTexture render(width, height);
  pangolin::GlRenderBuffer renderBuffer(width, height);
  pangolin::GlFramebuffer frameBuffer(render, renderBuffer);

  pangolin::GlTexture depthTexture(width, height, GL_R32F, false, 0, GL_RED, GL_FLOAT, 0);
  pangolin::GlFramebuffer depthFrameBuffer(depthTexture, renderBuffer);

  // beginning modified

  std::vector <CameraPose> poses = createPoses(textfile, renderDistance);
  size_t numFrames = poses.size();


  //MOVED OUT OF LOOP
  // And move to the left
  Eigen::Matrix4d T_new_old = Eigen::Matrix4d::Identity();

  T_new_old.topRightCorner(3, 1) = Eigen::Vector3d(0.025, 0, 0);

  // load mirrors
  std::vector<MirrorSurface> mirrors;
  if (surfaceFile.length()) {
    std::ifstream file(surfaceFile);
    picojson::value json;
    picojson::parse(json, file);

    for (size_t i = 0; i < json.size(); i++) {
      mirrors.emplace_back(json[i]);
    }
    std::cout << "Loaded " << mirrors.size() << " mirrors" << std::endl;
  }

  // load mesh and textures
  PTexMesh ptexMesh(meshFile, atlasFolder);

  pangolin::ManagedImage<Eigen::Matrix<uint8_t, 3, 1>> image(width, height);
  pangolin::ManagedImage<float> depthImage(width, height);
  pangolin::ManagedImage<uint16_t> depthImageInt(width, height);


  for (size_t i = 0; i < numFrames; i++) {

    CameraPose pose = poses[i];
    // Setup a camera
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrixRDF_BottomLeft(
            width,
            height,
            width / 2.0f,
            width / 2.0f,
            (width - 1.0f) / 2.0f,
            (height - 1.0f) / 2.0f,
            0.1f,
            100.0f),
            //modifying the pose
        pangolin::ModelViewLookAtRDF(
            pose.ex, pose.ey, pose.ez, // camera position
            pose.lx, pose.ly, pose.lz, // where camera is looking at
            pose.ux, pose.uy, pose.uz) // camera is looking up (010) or down (0-10)
      );

    // Start at some origin
    Eigen::Matrix4d T_camera_world = s_cam.GetModelViewMatrix();

    //MOVED OUT OF LOOP

    const std::string shadir = STR(SHADER_DIR);
    MirrorRenderer mirrorRenderer(mirrors, width, height, shadir);

    std::cout << "\rRendering frame " << i + 1 << "/" << numFrames << "... ";
    std::cout.flush();

    // Render
    frameBuffer.Bind();
    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, width, height);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_CULL_FACE);

    ptexMesh.Render(s_cam);

    glDisable(GL_CULL_FACE);

    glPopAttrib(); //GL_VIEWPORT_BIT
    frameBuffer.Unbind();

    for (size_t i = 0; i < mirrors.size(); i++) {
      MirrorSurface& mirror = mirrors[i];
      // capture reflections
      mirrorRenderer.CaptureReflection(mirror, ptexMesh, s_cam, frontFace);

      frameBuffer.Bind();
      glPushAttrib(GL_VIEWPORT_BIT);
      glViewport(0, 0, width, height);

      // render mirror
      mirrorRenderer.Render(mirror, mirrorRenderer.GetMaskTexture(i), s_cam);

      glPopAttrib(); //GL_VIEWPORT_BIT
      frameBuffer.Unbind();
    }

    // Download and save
    render.Download(image.ptr, GL_RGB, GL_UNSIGNED_BYTE);
    //cube_##_##_depth
    // char filename[1000];
    // snprintf(filename, 1000, "frame%06zu.png", i);
    std::string cur_file;
    cur_file = std::format("{}_frame.png",pose.filename);

    pangolin::SaveImage(
        image.UnsafeReinterpret<uint8_t>(),
        pangolin::PixelFormatFromString("RGB24"),
        cur_file);

    if (renderDepth) {
      // render depth
      depthFrameBuffer.Bind();
      glPushAttrib(GL_VIEWPORT_BIT);
      glViewport(0, 0, width, height);
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

      glEnable(GL_CULL_FACE);

      ptexMesh.RenderDepth(s_cam, depthScale);

      glDisable(GL_CULL_FACE);

      glPopAttrib(); //GL_VIEWPORT_BIT
      depthFrameBuffer.Unbind();

      depthTexture.Download(depthImage.ptr, GL_RED, GL_FLOAT);

      // convert to 16-bit int
      for(size_t i = 0; i < depthImage.Area(); i++)
          depthImageInt[i] = static_cast<uint16_t>(depthImage[i] + 0.5f);


      cur_file = std::format("{}_depth.png",pose.filename);

      //snprintf(filename, 1000, "depth%06zu.png", i);
      pangolin::SaveImage(
          depthImageInt.UnsafeReinterpret<uint8_t>(),
          pangolin::PixelFormatFromString("GRAY16LE"),
          cur_file, true, 34.0f);
    }

    // Move the camera
    T_camera_world = T_camera_world * T_new_old.inverse();

    s_cam.GetModelViewMatrix() = T_camera_world;
  }
  std::cout << "\rRendering frame " << numFrames << "/" << numFrames << "... done" << std::endl;

  return 0;
}

