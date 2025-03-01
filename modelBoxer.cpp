#include "globals.hpp"

// Model Variables
const char* objFilePath = "./boxer_real.obj";
const aiScene* scene;
Assimp::Importer importer;

void setNodeColor(const std::string& nodeName) {
    if (hitTimer2 > 0.0f) {
        glColor3f(1.0f, 0.0f, 0.0f); // red tint
        return;
    }
    if (nodeName == "upper_arm_L" || nodeName == "upper_arm_R") glColor3f(1.0f, 0.8f, 0.6f); // Red
    else if (nodeName == "bicep_L" || nodeName == "bicep_R") glColor3f(1.0f, 0.8f, 0.6f); // Green
    else if (nodeName == "hand_L" || nodeName == "hand_R") glColor3f(1.0f, 0.0f, 0.0f); // Blue
    else if (nodeName == "gyatt_L" || nodeName == "gyatt_R") glColor3f(1.0f, 0.0f, 0.0f); // Yellow
    else if (nodeName == "leg_L" || nodeName == "leg_R") glColor3f(1.0f, 0.8f, 0.6f); // Orange
    else if (nodeName == "foot_L" || nodeName == "foot_R") glColor3f(0.5f, 0.0f, 0.1f); // Purple
    else if (nodeName == "head") glColor3f(1.0f, 0.8f, 0.6f); // Dark brown
    else if (nodeName == "neck") glColor3f(1.0f, 0.8f, 0.6f); // Light skin tone
    else if (nodeName == "torso") glColor3f(1.0f, 0.7f, 0.4f); // Brown
    else glColor3f(0.8f, 0.8f, 0.8f); // Default gray
}

// Node-specific offset variables
struct NodeOffset {
    float x, y, z;
};

// Define joint pivot offsets for specific nodes
std::map<std::string, NodeOffset> jointOffsets = {
    {"upper_arm_L", {0.2f, 0.5f, 0.0f}},
    {"bicep_L", {0.2f, 0.5f, 0.0f}},
    {"hand_L", {0.2f, 0.5f, 0.0f}}
    // Add more node-specific offsets as needed
};

// Model Boxer
void renderNode(aiNode* node) {
    std::string nodeName = node->mName.C_Str();
    NodeOffset offset = {0.0f, 0.0f, 0.0f};
    
    // Get node-specific offsets if available
    if (jointOffsets.find(nodeName) != jointOffsets.end()) {
        offset = jointOffsets[nodeName];
    }

    // Rotate from shoulder joint
    if (nodeName == "upper_arm_L") {
        glPushMatrix();
        customTranslatef(offset.x, offset.y, offset.z);  // Move to pivot point
        customRotatef(-armRotation, 0, 1, 0);
        customTranslatef(-offset.x, -offset.y, -offset.z);  // Move back
    }

    // Rotate from elbow joint
    if (nodeName == "bicep_L") {
        glPushMatrix();
        customTranslatef(offset.x, offset.y, offset.z);  // Move to pivot point
        customRotatef(-bicepRotation, 0, 1, 0);  // Rotate around Y axis
        customTranslatef(-offset.x, -offset.y, -offset.z);  // Move back
    }

    // Rotate from wrist joint
    if (nodeName == "hand_L") {
        glPushMatrix();
        customTranslatef(offset.x, offset.y, offset.z);  // Move to pivot point
        customRotatef(-handRotation, 0, 1, 0);
        customTranslatef(-offset.x, -offset.y, -offset.z);  // Move back
    }

    setNodeColor(nodeName); // Apply color

    // Draw mesh
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        glBegin(GL_TRIANGLES);
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                int index = face.mIndices[k];
                aiVector3D vertex = mesh->mVertices[index];
                glVertex3f(vertex.x, vertex.y, vertex.z);
            }
        }
        glEnd();
    }

    // Recursively render children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        renderNode(node->mChildren[i]);
    }

    if (nodeName == "upper_arm_L" || nodeName == "bicep_L" || nodeName == "hand_L") {
        glPopMatrix();
    }
}

void loadModel() {
    scene = importer.ReadFile(objFilePath, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error loading OBJ: " << importer.GetErrorString() << std::endl;
        exit(EXIT_FAILURE);
    }
}