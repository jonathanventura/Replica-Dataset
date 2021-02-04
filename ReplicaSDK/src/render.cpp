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

class InvalidFormatException: public exception
{
  virtual const char* what() const throw()
  {
    return "The camera poses file is formatted invalidly";
  }
} InvalidFormatEx;


class CameraPose{

  public:
    GLPrecision ex, ey, ez, lx, ly, lz, ux, uy, uz;
    CameraPose(
        GLPrecision e_x, 
        GLPrecision e_y, 
        GLPrecision e_z, 
        GLPrecision l_x, 
        GLPrecision l_y, 
        GLPrecision l_z, 
        GLPrecision u_x, 
        GLPrecision u_y, 
        GLPrecision u_z);
    
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
        GLPrecision u_z)
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
}

// hold off until format decided
// void check_valid_line(std::string line){

//   std::string val;
//   int count, period_count = 0;
//   std::istringstream iss (line);

//   while(iss >> val)
//   {
//     period_count = 0
//     std::string::const_iterator iter = val.begin();
//     while (iter != val.end() && (std::isdigit(*iter) || *iter == ".")){
//             ++iter;
//             if (*iter == ".") {
//               ++period_count;
//               if(period_count)
//             }
//     } 

//     ++count;
//   }

//   if (count != 9){
//       throw InvalidFormatEx;
//   }
// }

// old function

// std::vector <CameraPose> createPoses(std::string textfile)
// {
//   std::string line;
//   GLPrecision ex, ey, ez, lx, ly, lz, ux, uy, uz;
//   std::vector <CameraPose> poses;
  
//   std::ifstream file(textfile);
  
//   while (getline(file, line))
//   {

    
//     check_valid_line(line);

//     std::istringstream iss (line);
//     iss >> ex >> ey >> ez >> lx >> ly >> lz >> ux >> uy >> uz;
//     // assumes poses per line, no error handling atm
//     poses.push_back(CameraPose(ex, ey, ez, lx, ly, lz, ux, uy, uz)); 
//   }

//   return poses;
// }


// now assumes origin point,, and orientation is 0 0 1, 
// might modify to remove orientation entirely, and modify file names
std::vector <CameraPose> createPoses(std::string textfile, float dist)
{
  std::string line;
  GLPrecision ex, ey, ez, lx, ly, lz, ux, uy, uz;
  std::vector <CameraPose> poses;
  
  std::ifstream file(textfile);

  while (getline(file, line))
  {
    //check_valid_line(line);
    std::istringstream iss (line);
    iss >> ex >> ey >> ez >> lx >> ly >> lz >> ux >> uy >> uz;
    // assumes poses per line, no error handling atm
    poses.push_back(CameraPose(ex, ey, ez, lx, ly + dist, lz, ux, uy, uz)); 
    poses.push_back(CameraPose(ex, ey, ez, lx, ly - dist, lz, ux, uy, uz));

    poses.push_back(CameraPose(ex, ey, ez, lx + dist, ly, lz, ux, uy, uz));
    poses.push_back(CameraPose(ex, ey, ez, lx - dist, ly, lz, ux, uy, uz));

    poses.push_back(CameraPose(ex, ey, ez, lx, ly, lz + dist, ux, uy, uz));
    poses.push_back(CameraPose(ex, ey, ez, lx, ly, lz - dist, ux, uy, uz));
  }

  return poses;
}
// added for convenience

int main(int argc, char* argv[]) {
// original
//   ASSERT(argc == 3 || argc == 4, "Usage: ./ReplicaRenderer mesh.ply /path/to/atlases [mirrorFile]");

//   const std::string meshFile(argv[1]);
//   const std::string atlasFolder(argv[2]);
//   ASSERT(pangolin::FileExists(meshFile));
//   ASSERT(pangolin::FileExists(atlasFolder));

//   std::string surfaceFile;
//   if (argc == 4) {
//     surfaceFile = std::string(argv[3]);
//     ASSERT(pangolin::FileExists(surfaceFile));
//   }



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

    const std::string shadir = STR(SHADER_DIR);
    MirrorRenderer mirrorRenderer(mirrors, width, height, shadir);

    // load mesh and textures
    PTexMesh ptexMesh(meshFile, atlasFolder);

    pangolin::ManagedImage<Eigen::Matrix<uint8_t, 3, 1>> image(width, height);
    pangolin::ManagedImage<float> depthImage(width, height);
    pangolin::ManagedImage<uint16_t> depthImageInt(width, height);

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

    char filename[1000];
    snprintf(filename, 1000, "frame%06zu.png", i);

    pangolin::SaveImage(
        image.UnsafeReinterpret<uint8_t>(),
        pangolin::PixelFormatFromString("RGB24"),
        std::string(filename));

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

      snprintf(filename, 1000, "depth%06zu.png", i);
      pangolin::SaveImage(
          depthImageInt.UnsafeReinterpret<uint8_t>(),
          pangolin::PixelFormatFromString("GRAY16LE"),
          std::string(filename), true, 34.0f);
    }

    // Move the camera
    T_camera_world = T_camera_world * T_new_old.inverse();

    s_cam.GetModelViewMatrix() = T_camera_world;
  }
  std::cout << "\rRendering frame " << numFrames << "/" << numFrames << "... done" << std::endl;
// end modified

//    original

// Setup a camera
  // pangolin::OpenGlRenderState s_cam(
  //     pangolin::ProjectionMatrixRDF_BottomLeft(
  //         width,
  //         height,
  //         width / 2.0f,
  //         width / 2.0f,
  //         (width - 1.0f) / 2.0f,
  //         (height - 1.0f) / 2.0f,
  //         0.1f,
  //         100.0f),
  //     pangolin::ModelViewLookAtRDF(0, 0, 4, 0, 0, 0, 0, 1, 0));

  // // Start at some origin
  // Eigen::Matrix4d T_camera_world = s_cam.GetModelViewMatrix();

  // // And move to the left
  // Eigen::Matrix4d T_new_old = Eigen::Matrix4d::Identity();

  // T_new_old.topRightCorner(3, 1) = Eigen::Vector3d(0.025, 0, 0);

  // // load mirrors
  // std::vector<MirrorSurface> mirrors;
  // if (surfaceFile.length()) {
  //   std::ifstream file(surfaceFile);
  //   picojson::value json;
  //   picojson::parse(json, file);

  //   for (size_t i = 0; i < json.size(); i++) {
  //     mirrors.emplace_back(json[i]);
  //   }
  //   std::cout << "Loaded " << mirrors.size() << " mirrors" << std::endl;
  // }

  // const std::string shadir = STR(SHADER_DIR);
  // MirrorRenderer mirrorRenderer(mirrors, width, height, shadir);

  // // load mesh and textures
  // PTexMesh ptexMesh(meshFile, atlasFolder);

  // pangolin::ManagedImage<Eigen::Matrix<uint8_t, 3, 1>> image(width, height);
  // pangolin::ManagedImage<float> depthImage(width, height);
  // pangolin::ManagedImage<uint16_t> depthImageInt(width, height);


//   // Render some frames
//   const size_t numFrames = 100;
//   for (size_t i = 0; i < numFrames; i++) {
//     std::cout << "\rRendering frame " << i + 1 << "/" << numFrames << "... ";
//     std::cout.flush();

//     // Render
//     frameBuffer.Bind();
//     glPushAttrib(GL_VIEWPORT_BIT);
//     glViewport(0, 0, width, height);
//     glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

//     glEnable(GL_CULL_FACE);

//     ptexMesh.Render(s_cam);

//     glDisable(GL_CULL_FACE);

//     glPopAttrib(); //GL_VIEWPORT_BIT
//     frameBuffer.Unbind();

//     for (size_t i = 0; i < mirrors.size(); i++) {
//       MirrorSurface& mirror = mirrors[i];
//       // capture reflections
//       mirrorRenderer.CaptureReflection(mirror, ptexMesh, s_cam, frontFace);

//       frameBuffer.Bind();
//       glPushAttrib(GL_VIEWPORT_BIT);
//       glViewport(0, 0, width, height);

//       // render mirror
//       mirrorRenderer.Render(mirror, mirrorRenderer.GetMaskTexture(i), s_cam);

//       glPopAttrib(); //GL_VIEWPORT_BIT
//       frameBuffer.Unbind();
//     }

//     // Download and save
//     render.Download(image.ptr, GL_RGB, GL_UNSIGNED_BYTE);

//     char filename[1000];
//     snprintf(filename, 1000, "frame%06zu.png", i);

//     pangolin::SaveImage(
//         image.UnsafeReinterpret<uint8_t>(),
//         pangolin::PixelFormatFromString("RGB24"),
//         std::string(filename));

//     if (renderDepth) {
//       // render depth
//       depthFrameBuffer.Bind();
//       glPushAttrib(GL_VIEWPORT_BIT);
//       glViewport(0, 0, width, height);
//       glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

//       glEnable(GL_CULL_FACE);

//       ptexMesh.RenderDepth(s_cam, depthScale);

//       glDisable(GL_CULL_FACE);

//       glPopAttrib(); //GL_VIEWPORT_BIT
//       depthFrameBuffer.Unbind();

//       depthTexture.Download(depthImage.ptr, GL_RED, GL_FLOAT);

//       // convert to 16-bit int
//       for(size_t i = 0; i < depthImage.Area(); i++)
//           depthImageInt[i] = static_cast<uint16_t>(depthImage[i] + 0.5f);

//       snprintf(filename, 1000, "depth%06zu.png", i);
//       pangolin::SaveImage(
//           depthImageInt.UnsafeReinterpret<uint8_t>(),
//           pangolin::PixelFormatFromString("GRAY16LE"),
//           std::string(filename), true, 34.0f);
//     }

//     // Move the camera
//     T_camera_world = T_camera_world * T_new_old.inverse();

//     s_cam.GetModelViewMatrix() = T_camera_world;
//   }
//   std::cout << "\rRendering frame " << numFrames << "/" << numFrames << "... done" << std::endl;

  return 0;
}

