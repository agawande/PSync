/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019,  The University of Memphis
 *
 * This file is part of PSync.
 * See AUTHORS.md for complete list of PSync authors and contributors.
 *
 * PSync is free software: you can redistribute it and/or modify it under the terms
 * of the GNU Lesser General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * PSync is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * PSync, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "PSync/detail/state.hpp"

#include <ndn-cxx/util/ostream-joiner.hpp>

namespace psync {

State::State(const ndn::Block& block)
{
  wireDecode(block);
}

void
State::addContent(const ndn::Name& prefix, std::shared_ptr<ndn::Block> block)
{
  m_contentWithBlock.emplace(prefix, block);
  m_wire.reset();
}

const ndn::Block&
State::wireEncode() const
{
  if (m_wire.hasWire()) {
    return m_wire;
  }

  ndn::EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  ndn::EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();
  return m_wire;
}

template<ndn::encoding::Tag TAG>
size_t
State::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;

  for (auto it = m_contentWithBlock.rbegin(); it != m_contentWithBlock.rend(); ++it) {
    if (it->second) {
      totalLength += block.prependBlock(ndn::Block(tlv::PSyncDataBlock, *it->second));
    }
    totalLength += it->first.wireEncode(block);
  }

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(tlv::PSyncContent);

  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(State);

void
State::wireDecode(const ndn::Block& wire)
{
  auto blockType = wire.type();
  if (blockType != tlv::PSyncContent) {
    BOOST_THROW_EXCEPTION(ndn::tlv::Error("Expected PSyncContent Block, but Block is of type: #" +
                                          ndn::to_string(blockType)));
  }

  wire.parse();
  m_wire = wire;

  auto mapIt = m_contentWithBlock.begin();
  for (auto it = m_wire.elements_begin(); it != m_wire.elements_end(); ++it) {
    if (it->type() == ndn::tlv::Name) {
      mapIt = m_contentWithBlock.emplace(ndn::Name(*it), nullptr).first;
    }
    else if (it->type() == tlv::PSyncDataBlock) {
      it->parse();
      mapIt->second = std::make_shared<ndn::Block>(it->value(), it->value_size());
    }
    else {
      BOOST_THROW_EXCEPTION(ndn::tlv::Error("Expected Name Block, but Block is of type: #" +
                                            ndn::to_string(it->type())));
    }
  }
}

std::ostream&
operator<<(std::ostream& os, const State& state)
{
  auto content = state.getContentWithBlock();
  auto it = content.begin();

  os << "[" << it->first;
  for (++it; it != content.end(); ++it) {
    os << ", " << it->first;
  }
  os << "]";

  return os;
}

bool
operator==(const State& state1, const State& state2)
{
  auto lhs = state1.getContentWithBlock();
  auto rhs = state2.getContentWithBlock();
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                    [] (auto a, auto b) { return a.first == b.first &&
                                                 ((a.second == nullptr && b.second == nullptr) ||
                                                 (*a.second == *b.second)); });
}

} // namespace psync
