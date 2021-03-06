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

#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/RigidBody.h"

namespace
{

#pragma pack(push, 1)

struct RigidBodyUnit
{
    uint8_t collisionGroupID;
    uint16_t collsionMask;
    uint8_t shapeType;
    float size[3];
    float position[3];
    float rotation[3];
    float mass;
    float linearDamping;
    float angularDamping;
    float restitution;
    float friction;
    uint8_t type;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

RigidBody::RigidBody()
    : internal::BaseRigidBody()
{
}

RigidBody::~RigidBody()
{
}

bool RigidBody::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size, boneIndexSize = info.boneIndexSize;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.rigidBodiesPtr = ptr;
    for (size_t i = 0; i < size; i++) {
        size_t nNameSize;
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        /* name in English */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        if (!internal::validateSize(ptr, boneIndexSize + sizeof(RigidBodyUnit), rest)) {
            return false;
        }
    }
    info.rigidBodiesCount = size;
    return true;
}

bool RigidBody::loadRigidBodies(const Array<RigidBody *> &rigidBodies, const Array<Bone *> &bones)
{
    const int nRigidBodies = rigidBodies.count();
    const int nbones = bones.count();
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = rigidBodies[i];
        const int boneIndex = rigidBody->m_boneIndex;
        if (boneIndex >= 0) {
            if (boneIndex >= nbones) {
                return false;
            }
            else {
                rigidBody->build(bones[boneIndex], i);
            }
        }
        else {
            rigidBody->build(NullBone::sharedReference(), i);
        }
    }
    return true;
}

size_t RigidBody::estimateTotalSize(const Array<RigidBody *> &rigidBodies, const Model::DataInfo &info)
{
    const int nbodies = rigidBodies.count();
    size_t size = 0;
    size += sizeof(nbodies);
    for (int i = 0; i < nbodies; i++) {
        RigidBody *body = rigidBodies[i];
        size += body->estimateSize(info);
    }
    return size;
}

void RigidBody::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    IEncoding *encoding = info.encoding;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_name);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_englishName);
    m_boneIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
    RigidBodyUnit unit;
    internal::getData(ptr, unit);
    m_collisionGroupID = unit.collisionGroupID;
    m_groupID = 0x0001 << m_collisionGroupID;
    m_collisionGroupMask = unit.collsionMask;
    m_shapeType = static_cast<ShapeType>(unit.shapeType);
    internal::setPositionRaw(unit.size, m_size);
    internal::setPosition(unit.position, m_position);
    internal::setPositionRaw(unit.rotation, m_rotation);
    m_mass = unit.mass;
    m_linearDamping = unit.linearDamping;
    m_angularDamping = unit.angularDamping;
    m_restitution = unit.restitution;
    m_friction = unit.friction;
    m_type = static_cast<ObjectType>(unit.type);
    ptr += sizeof(unit);
    size = ptr - start;
}

void RigidBody::write(uint8_t *data, const Model::DataInfo &info) const
{
    internal::writeString(m_name, info.codec, data);
    internal::writeString(m_englishName, info.codec, data);
    internal::writeSignedIndex(m_boneIndex, info.boneIndexSize, data);
    RigidBodyUnit rbu;
    rbu.angularDamping = m_angularDamping;
    rbu.collisionGroupID = m_collisionGroupID;
    rbu.collsionMask = m_collisionGroupMask;
    rbu.friction = m_friction;
    rbu.linearDamping = m_linearDamping;
    rbu.mass = m_mass;
    internal::getPosition(m_position, &rbu.position[0]);
    rbu.restitution = m_restitution;
    internal::getPositionRaw(m_rotation, &rbu.rotation[0]);
    rbu.shapeType = m_shapeType;
    internal::getPositionRaw(m_size, rbu.size);
    rbu.type = m_type;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&rbu), sizeof(rbu), data);
}

size_t RigidBody::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0;
    size += internal::estimateSize(m_name, info.codec);
    size += internal::estimateSize(m_englishName, info.codec);
    size += info.boneIndexSize;
    size += sizeof(RigidBodyUnit);
    return size;
}

void RigidBody::mergeMorph(const Morph::Impulse * /* morph */, const IMorph::WeightPrecision & /* weight */)
{
}

} /* namespace pmx */
} /* namespace vpvl2 */

