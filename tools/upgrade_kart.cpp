#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
namespace fs = std::filesystem;
using Index = std::uint32_t;
using Vertex = std::array<float, 13>;
using Finish = std::array<float, 7>;
constexpr Index triangleLimit = 150000;
constexpr float tau = 6.28318530718f;
static_assert(sizeof(Vertex) == 52 && std::numeric_limits<float>::is_iec559);

struct Vec {
    float x, y, z;
};
Vec operator+(Vec a, Vec b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec operator-(Vec a, Vec b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec operator*(Vec a, float s) { return {a.x * s, a.y * s, a.z * s}; }
Vec cross(Vec a, Vec b) {
    return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}
float length(Vec a) { return std::sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }
Vec unit(Vec a) { return a * (1.f / length(a)); }

struct Bounds {
    Vec lo{INFINITY, INFINITY, INFINITY}, hi{-INFINITY, -INFINITY, -INFINITY};
    void add(Vec p) {
        lo = {std::min(lo.x,p.x), std::min(lo.y,p.y), std::min(lo.z,p.z)};
        hi = {std::max(hi.x,p.x), std::max(hi.y,p.y), std::max(hi.z,p.z)};
    }
    Vec center() const { return (lo + hi) * .5f; }
    Vec size() const { return hi - lo; }
    bool contains(Vec p) const {
        return p.x >= lo.x && p.x <= hi.x && p.y >= lo.y && p.y <= hi.y &&
               p.z >= lo.z && p.z <= hi.z;
    }
};

Vec position(const Vertex& v) { return {v[0], v[1], v[2]}; }
struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<Index> indices;

    Index add(Vec p, Vec n, const Finish& f) {
        const auto index = static_cast<Index>(vertices.size());
        vertices.push_back({p.x,p.y,p.z,n.x,n.y,n.z,f[0],f[1],f[2],f[3],f[4],f[5],f[6]});
        return index;
    }
    void quad(Index a, Index b, Index c, Index d) {
        indices.insert(indices.end(), {a,b,c,a,c,d});
    }
};

Mesh readMesh(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    char magic[4]{};
    Index nv{}, ni{};
    in.read(magic, 4);
    in.read(reinterpret_cast<char*>(&nv), sizeof(nv));
    in.read(reinterpret_cast<char*>(&ni), sizeof(ni));
    if (!in || std::memcmp(magic, "PHXM", 4) || !nv || nv > 2000000 ||
        !ni || ni % 3 || ni / 3 >= triangleLimit ||
        fs::file_size(path) != 12ull + 52ull * nv + 4ull * ni) {
        throw std::runtime_error("Invalid PHXM header, counts or file size");
    }
    Mesh mesh;
    mesh.vertices.resize(nv);
    mesh.indices.resize(ni);
    in.read(reinterpret_cast<char*>(mesh.vertices.data()), 52ull * nv);
    in.read(reinterpret_cast<char*>(mesh.indices.data()), 4ull * ni);
    if (!in || *std::max_element(mesh.indices.begin(), mesh.indices.end()) >= nv)
        throw std::runtime_error("Truncated PHXM or out-of-range index");
    for (const auto& v : mesh.vertices)
        for (float value : v)
            if (!std::isfinite(value)) throw std::runtime_error("Non-finite vertex value");
    return mesh;
}

void writeMesh(const fs::path& path, const Mesh& mesh) {
    const Index nv = static_cast<Index>(mesh.vertices.size());
    const Index ni = static_cast<Index>(mesh.indices.size());
    std::ofstream out(path, std::ios::binary);
    out.write("PHXM", 4);
    out.write(reinterpret_cast<const char*>(&nv), sizeof(nv));
    out.write(reinterpret_cast<const char*>(&ni), sizeof(ni));
    out.write(reinterpret_cast<const char*>(mesh.vertices.data()), 52ull * nv);
    out.write(reinterpret_cast<const char*>(mesh.indices.data()), 4ull * ni);
    out.close();
    if (!out) throw std::runtime_error("Could not write PHXM");
}

struct Region {
    Bounds bounds;
    Finish finish{};
};

Region measure(const Mesh& base, Bounds window, float group, float metal) {
    Region region;
    for (const auto& vertex : base.vertices) {
        const Vec p = position(vertex);
        if (vertex[11] == group && std::abs(vertex[9] - metal) < .001f &&
            window.contains(p)) {
            region.bounds.add(p);
            std::copy_n(vertex.begin() + 6, 7, region.finish.begin());
        }
    }
    const Vec size = region.bounds.size();
    if (!(size.x > 0 && size.y > 0 && size.z > 0))
        throw std::runtime_error("Missing measured MK-III mounting region");
    return region;
}

struct Axis {
    Vec along, u, v;
    explicit Axis(Vec direction) : along(unit(direction)) {
        const Vec reference = std::abs(along.z) > .9f ? Vec{0,1,0} : Vec{0,0,1};
        u = unit(cross(reference, along));
        v = cross(along, u);
    }
    Vec radial(float angle) const { return u * std::cos(angle) + v * std::sin(angle); }
};

struct Section {
    float offset, radius;
};

// Revolve a closed profile. Each profile edge has its own crease normals,
// while shared vertices around each ring give smooth circumferential normals.
void lathe(Mesh& mesh, Vec origin, const Axis& axis, int sides,
           std::initializer_list<Section> profile, const Finish& finish) {
    for (auto edge = profile.begin(); edge != profile.end(); ++edge) {
        const Section a = *edge;
        const Section b = edge + 1 == profile.end() ? *profile.begin() : *(edge + 1);
        const Index first = static_cast<Index>(mesh.vertices.size());
        for (int j = 0; j < sides; ++j) {
            const Vec r = axis.radial(tau * j / sides);
            const Vec n = unit(r * (b.offset - a.offset) - axis.along * (b.radius - a.radius));
            mesh.add(origin + axis.along*a.offset + r*a.radius, n, finish);
            mesh.add(origin + axis.along*b.offset + r*b.radius, n, finish);
        }
        for (int j = 0; j < sides; ++j) {
            const Index c = first + 2*j, next = first + 2*((j + 1) % sides);
            mesh.quad(c, next, next + 1, c + 1);
        }
    }
}

void spring(Mesh& mesh, Vec start, Vec end, float radius, const Finish& finish) {
    const Axis axis(end - start);
    constexpr int turns = 6, steps = turns * 32, sides = 12;
    const float wire = radius * .13f;
    const Index first = static_cast<Index>(mesh.vertices.size());
    for (int j = 0; j <= steps; ++j) {
        const float t = float(j) / steps;
        const float angle = tau * turns * t;
        const Vec r = axis.radial(angle);
        const Vec tangent = unit(end - start + axis.radial(angle + tau/4) * (radius*tau*turns));
        const Vec binormal = cross(tangent, r);
        const Vec p = start + (end - start)*t + r*radius;
        for (int k = 0; k < sides; ++k) {
            const float phase = tau*k/sides;
            const Vec n = r*std::cos(phase) + binormal*std::sin(phase);
            mesh.add(p + n*wire, n, finish);
        }
    }
    for (int j = 0; j < steps; ++j)
        for (int k = 0; k < sides; ++k) {
            const Index a = first + j*sides + k;
            const Index b = first + j*sides + (k+1)%sides;
            mesh.quad(a, b, b+sides, a+sides);
        }
}

void box(Mesh& mesh, Vec center, Vec x, Vec y, Vec z, const Finish& finish) {
    const std::array<Vec, 3> axes{x,y,z};
    for (int j = 0; j < 3; ++j) {
        const Vec u = axes[(j+1)%3], v = axes[(j+2)%3];
        for (float sign : {-1.f, 1.f}) {
            const Vec p = center + axes[j]*sign, n = unit(cross(u,v))*sign;
            const Vec w = v*sign;
            const Index a = mesh.add(p-u-w, n, finish);
            mesh.add(p+u-w, n, finish);
            mesh.add(p+u+w, n, finish);
            mesh.add(p-u+w, n, finish);
            mesh.quad(a,a+1,a+2,a+3);
        }
    }
}

void runningGear(Mesh& mesh, const Mesh& base, float station, float side) {
    const bool left = side < 0;
    const auto wheel = measure(base, {{station-.17f,left?-.80f:.70f,.13f},
                                     {station+.17f,left?-.70f:.80f,.43f}}, 3, .83f);
    const auto shock = measure(base, {{station-.09f,left?-.53f:.31f,.294f},
                                     {station+.14f,left?-.31f:.53f,.486f}}, 3, .78f);
    const auto tire = measure(base, {{station-.28f,left?-.80f:.55f,0.f},
                                    {station+.28f,left?-.55f:.80f,.56f}}, 4, .02f);
    const Vec hub = wheel.bounds.center();
    const float r = wheel.bounds.size().z * .5f;
    const Axis axle({0,side,0});
    const Vec outer{hub.x, left ? tire.bounds.lo.y : tire.bounds.hi.y, hub.z};
    lathe(mesh, outer, axle, 48, {{-.029f,r*.40f},{.007f,r*.40f},
          {.016f,r*.32f},{.016f,r*.12f},{-.029f,r*.12f}}, wheel.finish);
    lathe(mesh, outer, axle, 48, {{-.03f,r*.18f},{.022f,r*.18f},
          {.027f,r*.125f},{.027f,r*.03f},{-.03f,r*.03f}}, shock.finish);

    // The exposed twin friction rings sit inboard of the existing wheel.
    const float innerWheel = left ? tire.bounds.hi.y : tire.bounds.lo.y;
    const Vec rotor{hub.x, innerWheel - side*.026f, hub.z};
    for (float face : {-.008f,.008f}) {
        lathe(mesh, rotor + axle.along*face, axle, 64,
              {{-.0035f,r*.88f},{.0035f,r*.88f},{.0035f,r*.49f},{-.0035f,r*.49f}}, wheel.finish);
    }
    for (int j = 0; j < 16; ++j) {
        const float angle = tau*j/16;
        const Vec radial = axle.radial(angle), across = cross(axle.along, radial);
        box(mesh, rotor + radial*(r*.67f), radial*(r*.17f), across*.005f, axle.along*.006f, shock.finish);
    }

    for (int j = 0; j < 5; ++j) {
        const Vec lug = outer + axle.radial(tau*j/5)*(r*.275f);
        lathe(mesh, lug, axle, 6, {{.009f,.010f},{.020f,.010f},
              {.023f,.007f},{.023f,.003f},{.009f,.003f}}, shock.finish);
    }

    // The existing damper leans outward and toward negative X.
    const Vec c = shock.bounds.center(), d = shock.bounds.size();
    const Vec travel{-d.x*.30f, side*d.y*.64f, d.z*.69f};
    const Vec start = c - travel*.5f, end = c + travel*.5f;
    const float radius = d.x*.32f;
    spring(mesh, start, end, radius, shock.finish);
    const Axis damper(travel);
    for (Vec seat : {start,end})
        lathe(mesh, seat, damper, 32, {{-.007f,radius*1.19f},{.007f,radius*1.19f},
              {.009f,radius},{.009f,radius*.43f},{-.007f,radius*.43f}}, wheel.finish);
}

float seatSurface(const Mesh& base, float x, float y) {
    float height = -INFINITY;
    for (std::size_t i = 0; i < base.indices.size(); i += 3) {
        const auto& vertex = base.vertices[base.indices[i]];
        if (vertex[11] != 2 || std::abs(vertex[9] - .10f) >= .001f) continue;
        const Vec a = position(vertex);
        const Vec u = position(base.vertices[base.indices[i+1]]) - a;
        const Vec v = position(base.vertices[base.indices[i+2]]) - a;
        const float area = cross(u,v).z;
        if (std::abs(area) < 1e-8f) continue;
        const Vec p{x-a.x,y-a.y,0};
        const float s = cross(p,v).z / area, t = cross(u,p).z / area;
        if (s >= 0 && t >= 0 && s+t <= 1)
            height = std::max(height, a.z + s*u.z + t*v.z);
    }
    if (!std::isfinite(height)) throw std::runtime_error("Missing seat surface at harness mount");
    return height;
}

void engineAndHarness(Mesh& mesh, const Mesh& base) {
    const Region fins = measure(base, {{-1.028f,-.276f,.487f},{-.635f,-.234f,.635f}}, 3, .83f);
    const Vec span = fins.bounds.size();
    const Vec engine = fins.bounds.center();
    for (int j = 0; j < 13; ++j) {
        const float x = fins.bounds.lo.x + span.x*(j+.5f)/13;
        box(mesh, {x,fins.bounds.lo.y-.013f,engine.z},
            {span.x*.013f,0,0},{0,span.y*.68f,0},{0,0,span.z*.43f},fins.finish);
    }

    const Region seat = measure(base, {{-.65f,-.29f,.37f},{-.11f,.29f,.98f}}, 2, .10f);
    for (float side : {-1.f,1.f}) {
        const Region strap = measure(base, {{-.56f,side<0?-.24f:.04f,.53f},
                                            {-.26f,side<0?-.04f:.24f,.94f}}, 2, .37f);
        const float x = strap.bounds.center().x + .015f;
        const float y = side*seat.bounds.size().y*.26f;
        // Place the adjuster on the exposed shell at each shoulder strap mount.
        const Vec width{0,1,0};
        const Vec along = unit({.024f,0,seatSurface(base,x+.012f,y)-seatSurface(base,x-.012f,y)});
        const Vec normal = cross(along,width);
        const Vec p = Vec{x,y,seatSurface(base,x,y)} + normal*.006f;
        for (float sign : {-1.f,1.f}) {
            box(mesh,p+width*(sign*.023f),along*.028f,width*.004f,normal*.005f,strap.finish);
            box(mesh,p+along*(sign*.023f),along*.005f,width*.019f,normal*.005f,strap.finish);
        }
        box(mesh,p,along*.004f,width*.019f,normal*.007f,strap.finish);
    }
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 3) throw std::runtime_error("Usage: upgrade_kart BASE.mesh NEW.mesh");
        const std::uint16_t byteOrder = 1;
        if (*reinterpret_cast<const unsigned char*>(&byteOrder) != 1)
            throw std::runtime_error("PHXM requires a little-endian host");

        const fs::path input = argv[1], output = argv[2];
        if (fs::exists(output)) throw std::runtime_error("Output must be a new file");
        const Mesh base = readMesh(input);
        // Guard the mounting windows against a different kart or a second upgrade.
        if (base.vertices.size() != 32946 || base.indices.size() != 185256)
            throw std::runtime_error("Expected the original 32946-vertex MK-III base");
        Mesh mesh = base;
        Bounds envelope;
        for (const auto& vertex : base.vertices) envelope.add(position(vertex));
        for (float station : {-.8f,.7f})
            for (float side : {-1.f,1.f}) runningGear(mesh, base, station, side);
        engineAndHarness(mesh, base);
        if (mesh.indices.size()/3 >= triangleLimit) throw std::runtime_error("Triangle budget exceeded");
        for (std::size_t j = base.vertices.size(); j < mesh.vertices.size(); ++j)
            if (!envelope.contains(position(mesh.vertices[j])))
                throw std::runtime_error("Detail exceeds the base mesh bounds");

        writeMesh(output, mesh);
        const Mesh verified = readMesh(output);
        if (verified.vertices.size() != mesh.vertices.size() || verified.indices != mesh.indices ||
            std::memcmp(verified.vertices.data(), mesh.vertices.data(), mesh.vertices.size()*sizeof(Vertex)))
            throw std::runtime_error("PHXM round-trip mismatch");
        std::cout << "Verified PHXM: " << verified.vertices.size() << " vertices, "
                  << verified.indices.size() << " indices, " << verified.indices.size()/3
                  << " triangles\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
