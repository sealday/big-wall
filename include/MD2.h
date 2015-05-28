struct MD2 {
    int magic;
    int version;
    int skinWidth;
    int skinHeight;
    int frameSize;
    int numSkins;
    int numVertices;
    int numTexCoords;
    int numTriangles;
    int numGlCommands;
    int numFrames;
    int offsetSkins;
    int offsetTexCoords;
    int offsetTriangles;
    int offsetFrames;
    int offsetGlCommands;
    int offsetEnd;
};

struct TriangleVertex {
    unsigned char vertex[3];
    unsigned char lightNormalIndex;
};

struct Frame {
    float scale[3];
    float translate[3];
    char name[16];
    TriangleVertex vertices[1];
};

struct Mesh {
    unsigned short meshIndex[3];     // indices to triangle vertices
    unsigned short stIndex[3];       // indices to texture coordinates
};

struct pcxHeader
{
    short id[2];
    short offset[2];
    short size[2];
};

struct MeshUV
{
    unsigned short s;
    unsigned short t;
};

struct Vertex {
    GLfloat coords[3];
};

struct VertexUV {
    GLfloat st[2];
};
