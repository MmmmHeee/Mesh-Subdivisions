
#include <fstream>
#include <cstring>
#include <iostream>

#include "obj_mesh.h"

static inline bool isSpace(const char c) {
  return (c == ' ') || (c == '\t');
}

static inline bool isNewLine(const char c) {
  return (c == '\r') || (c == '\n') || (c == '\0');
}

// Make index zero-base, and also support relative index. 
static inline int fixIndex(int idx, unsigned int n)
{
  int i;

  if (idx > 0) {
    i = idx - 1;
  } else if (idx == 0) {
    i = 0;
  } else { // negative value = relative
    i = n + idx;
  }
  return i;
}

static inline std::string parseString(const char*& token)
{
  std::string s;
  size_t b = strspn(token, " \t"); // find first character index that is not space
  size_t e = strcspn(token, " \t\r"); // find first character index that is space
  s = std::string(&token[b], &token[e]);

  token += (e - b);
  return s;
}

static inline int parseInt(const char*& token)
{
  token += strspn(token, " \t");
  int i = atoi(token);
  token += strcspn(token, " \t\r");
  return i;
}

static inline float parseFloat(const char*& token)
{
  token += strspn(token, " \t");
  float f = (float)atof(token);
  token += strcspn(token, " \t\r");
  return f;
}

static inline void parseFloat2(
  float& x, float& y,
  const char*& token)
{
  x = parseFloat(token);
  y = parseFloat(token);
}

static inline void parseFloat3(
  float& x, float& y, float& z,
  const char*& token)
{
  x = parseFloat(token);
  y = parseFloat(token);
  z = parseFloat(token);
}

// Parse triples: i, i/j/k, i//k, i/j
static vertex_index parseTriple(
  const char* &token,
  size_t vsize,
  size_t vnsize,
  size_t vtsize)
{
    vertex_index vi;
    vi.v_idx = -1; vi.vn_idx = -1; vi.vt_idx = -1;

    vi.v_idx = fixIndex(atoi(token), (unsigned int) vsize); // vertex position index
    token += strcspn(token, "/ \t\r");
    if (token[0] != '/') {
      return vi;
    }
    token++;

    // i//k i//
    if (token[0] == '/') {
      token++;
      if (!isNewLine(token[0])) {
        vi.vn_idx = fixIndex(atoi(token), (unsigned int) vnsize); // vertex normal index
        token += strcspn(token, "/ \t\r");
      }
      return vi;
    }
    
    // i/j/k or i/j
    vi.vt_idx = fixIndex(atoi(token), (unsigned int) vtsize); // vertex texture coordinate index
    token += strcspn(token, "/ \t\r");
    if (token[0] != '/') {
      return vi;
    }

    // i/j/k
    token++;  // skip '/'
    vi.vn_idx = fixIndex(atoi(token), (unsigned int) vnsize);
    token += strcspn(token, "/ \t\r");
    return vi; 
}

// load obj mesh from file
bool loadObj(std::string _filepath, obj_mesh & _obj) {
    std::ifstream ifs(_filepath);
    if (!ifs) {
        std::cout << "cannot load obj file: " << _filepath << std::endl;
        return false;
    }
    
    _obj.positions.clear();
    _obj.normals.clear();
    _obj.texcoords.clear();
    _obj.faces.clear();

    int maxchars = 8192;  // Alloc enough size.
    std::vector<char> buf(maxchars);  // Alloc enough size.
    while (ifs.peek() != -1) {
        ifs.getline(&buf[0], maxchars);

        std::string linebuf(&buf[0]);

        // Trim newline '\r\n' or '\n'
        if (linebuf.size() > 0) {
            if (linebuf[linebuf.size()-1] == '\n') linebuf.erase(linebuf.size()-1);
        }
        if (linebuf.size() > 0) {
            if (linebuf[linebuf.size()-1] == '\r') linebuf.erase(linebuf.size()-1);
        }

        // Skip if empty line.
        if (linebuf.empty()) {
            continue;
        }

        // Skip leading space.
        const char* token = linebuf.c_str();
        token += strspn(token, " \t");

        assert(token);
        if (token[0] == '\0') continue; // empty line
        if (token[0] == '#') continue;  // comment line

        // vertex
        if (token[0] == 'v' && isSpace((token[1]))) {
            token += 2;
            float x, y, z;
            parseFloat3(x, y, z, token);
            _obj.positions.push_back(glm::vec3(x, y, z));
            continue;
        }

        // normal
        if (token[0] == 'v' && token[1] == 'n' && isSpace((token[2]))) {
            token += 3;
            float x, y, z;
            parseFloat3(x, y, z, token);
            _obj.normals.push_back(glm::vec3(x, y, z));
            continue;
        }

        // _obj.texcoords
        if (token[0] == 'v' && token[1] == 't' && isSpace((token[2]))) {
            token += 3;
            float x, y;
            parseFloat2(x, y, token);
            _obj.texcoords.push_back(glm::vec2(x, y));
            continue;
        }

        // face
        if (token[0] == 'f' && isSpace((token[1]))) {
            token += 2;
            token += strspn(token, " \t");

            face_t face;
            while (!isNewLine(token[0])) {
                vertex_index vi = parseTriple(token, _obj.positions.size(), _obj.normals.size(), _obj.texcoords.size());
                face.push_back(std::move(vi));
                size_t n = strspn(token, " \t\r");
                token += n;
            }

            _obj.faces.push_back(std::move(face));
            continue;
        }
    }
    return true;
}

// write the obj mesh into file
bool writeObj(std::string _filepath, obj_mesh _obj) {
    std::ofstream ofs(_filepath);
    if (!ofs) {
        std::cout << "cannot write obj file: " << _filepath << std::endl;
        return false;
    }

    // vertex pos
    for (auto iter = _obj.positions.cbegin(); iter != _obj.positions.end(); iter++) {
        ofs << "v " << (*iter)[0] << " " 
                    << (*iter)[1] << " "
                    << (*iter)[2] << std::endl;
    }

    // vertex normal
    for (auto iter = _obj.normals.cbegin(); iter != _obj.normals.end(); iter++) {
        ofs << "vn " << (*iter)[0] << " " 
                     << (*iter)[1] << " " 
                     << (*iter)[2] << std::endl;
    }

    // vertex _obj.texcoords
    for (auto iter = _obj.texcoords.cbegin(); iter != _obj.texcoords.end(); iter++) {
        ofs << "vt " << (*iter)[0] << " " 
                     << (*iter)[1] << std::endl;
    }

    // face indices
    for (auto iter = _obj.faces.cbegin(); iter != _obj.faces.end(); iter++) {
        ofs << "f";
        // index start from zero, so add 1 when saved to file
        for (auto vi = iter->cbegin(); vi != iter->end(); vi++) {
            ofs << " "
                << ((vi->v_idx != -1) ? std::to_string(vi->v_idx + 1) : "") << "/"
                << ((vi->vt_idx != -1) ? std::to_string(vi->vt_idx + 1) : "") << "/"
                << ((vi->vn_idx != -1) ? std::to_string(vi->vn_idx + 1) : "");
        }
        ofs << std::endl;
    }

    return true;
}