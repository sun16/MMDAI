/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"
#include "vpvl2/pmd2/Morph.h"
#include "vpvl2/pmd2/Vertex.h"

namespace
{

using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct VertexMorphUnit {
    int vertexIndex;
    float position[3];
};

struct MorphUnit {
    uint8_t name[Morph::kNameSize];
    int nvertices;
    uint8_t type;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd2
{

const int Morph::kNameSize;

Morph::Morph(IEncoding *encodingRef)
    : m_encodingRef(encodingRef),
      m_name(0),
      m_category(kBase),
      m_weight(0),
      m_index(-1)
{
}

Morph::~Morph()
{
    delete m_name;
    m_name = 0;
    m_category = kBase;
    m_weight = 0;
    m_index = -1;
}

bool Morph::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size16(ptr, rest, size)) {
        return false;
    }
    info.morphsCount = size;
    info.morphsPtr = ptr;
    MorphUnit unit;
    VertexMorphUnit vunit;
    size_t unitSize = 0;
    for (size_t i = 0; i < size; i++) {
        if (sizeof(unit) > rest) {
            return false;
        }
        internal::getData(ptr, unit);
        unitSize = sizeof(unit) + unit.nvertices * sizeof(vunit);
        if (unitSize > rest) {
            return false;
        }
        internal::readBytes(unitSize, ptr, rest);
    }
    return true;
}

bool Morph::loadMorphs(const Array<Morph *> &morphs, const Array<Vertex *> &vertices)
{
    const int nmorphs = morphs.count(), nvertices = vertices.count();
    Morph *baseMorph = 0;
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        if (morph->category() == kBase) {
            const Array<Vector4> &morphVertices = morph->m_vertices;
            const int nMorphVertices = morphVertices.count();
            Array<Vertex *> &vertexRefs = morph->m_vertexRefs;
            for (int j = 0; j < nMorphVertices; j++) {
                const Vector4 &morphVertex = morphVertices[j];
                int vertexId = morphVertex.w();
                if (internal::checkBound(vertexId, 0, nvertices)) {
                    Vertex *vertex = vertices[vertexId];
                    vertex->setOrigin(morphVertex);
                    vertexRefs.add(vertex);
                }
            }
            baseMorph = morph;
            break;
        }
    }
    if (baseMorph) {
        for (int i = 0; i < nmorphs; i++) {
            Morph *morph = morphs[i];
            morph->m_index = i;
            if (morph->category() != kBase) {
                const Array<Vector4> &baseVertices = baseMorph->m_vertices;
                Array<Vector4> &morphVertices = morph->m_vertices;
                Array<Vertex *> &vertexRefs = morph->m_vertexRefs;
                const int nMorphVertices = morphVertices.count(), nBaseVertices = baseVertices.count();
                for (int j = 0; j < nMorphVertices; j++) {
                    Vector4 &morphVertex = morphVertices[j];
                    int vertexId = morphVertex.w();
                    if (internal::checkBound(vertexId, 0, nBaseVertices)) {
                        int baseVertexId = baseVertices[vertexId].w();
                        morphVertex.setW(baseVertexId);
                        vertexRefs.add(vertices[baseVertexId]);
                    }
                }
            }
        }
    }
    return true;
}

size_t Morph::estimateTotalSize(const Array<Morph *> &morphs, const Model::DataInfo &info)
{
    const int nmorphs = morphs.count();
    size_t size = 0;
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        size += morph->estimateSize(info);
    }
    return size;
}

void Morph::read(const uint8_t *data, size_t &size)
{
    MorphUnit unit;
    VertexMorphUnit vunit;
    Vector4 position;
    internal::getData(data, unit);
    uint8_t *ptr = const_cast<uint8_t *>(data + sizeof(unit));
    int nMorphVertices = unit.nvertices;
    for (int i = 0; i < nMorphVertices; i++) {
        internal::getData(ptr, vunit);
        if (internal::checkBound(vunit.vertexIndex, 0, 65535)) {
            internal::setPosition(vunit.position, position);
            position.setW(vunit.vertexIndex);
            m_vertices.add(position);
        }
        ptr += sizeof(vunit);
    }
    m_name = m_encodingRef->toString(unit.name, IString::kShiftJIS, kNameSize);
    m_category = static_cast<Category>(unit.type);
    size = ptr - data;
}

size_t Morph::estimateSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    size += sizeof(MorphUnit);
    size += m_vertices.count() * sizeof(VertexMorphUnit);
    return size;
}

void Morph::write(uint8_t *data, const Model::DataInfo & /* info */) const
{
    MorphUnit unit;
    uint8_t *name = m_encodingRef->toByteArray(m_name, IString::kShiftJIS);
    internal::copyBytes(unit.name, name, sizeof(unit.name));
    m_encodingRef->disposeByteArray(name);
    unit.nvertices = m_vertices.count();
    unit.type = m_category;
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&unit), sizeof(unit));
    data += sizeof(unit);
    VertexMorphUnit vunit;
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        const Vector4 &vertex = m_vertices[i];
        vunit.vertexIndex = vertex.w();
        internal::getPosition(vertex, vunit.position);
        internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&vunit), sizeof(vunit));
        data += sizeof(vunit);
    }
}

IMorph::Category Morph::category() const
{
    return m_category;
}

IMorph::Type Morph::type() const
{
    return kVertex;
}

bool Morph::hasParent() const
{
    return false;
}

const IMorph::WeightPrecision &Morph::weight() const
{
    return m_weight;
}

void Morph::setWeight(const WeightPrecision &value)
{
    const int nvertices = m_vertexRefs.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = m_vertexRefs[i];
        const Vector3 &v = m_vertices[i];
        vertex->mergeMorph(v, value);
    }
    m_weight = value;
}

void Morph::setIndex(int value)
{
    m_index = value;
}

}
}
