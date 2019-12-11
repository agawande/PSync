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

#ifndef PSYNC_STATE_HPP
#define PSYNC_STATE_HPP

#include <ndn-cxx/name.hpp>

namespace psync {

namespace tlv {

enum {
  PSyncContent = 128,
  PSyncDataBlock = 129
};

} // namespace tlv

/**
 * @brief State = tlv::PSyncContent TLV-LENGTH *(Name [PSyncDataBlock])
 *
 * PSyncDataBlock = tlv::PSyncDataBlock TLV-LENGTH Block
 *
 */
class State
{
public:
  explicit State(const ndn::Block& block);

  State() = default;

  void
  addContent(const ndn::Name& prefix, std::shared_ptr<ndn::Block> = nullptr);

  const std::map<ndn::Name, std::shared_ptr<ndn::Block>>&
  getContentWithBlock() const
  {
    return m_contentWithBlock;
  }

  const ndn::Block&
  wireEncode() const;

  template<ndn::encoding::Tag TAG>
  size_t
  wireEncode(ndn::EncodingImpl<TAG>& block) const;

  void
  wireDecode(const ndn::Block& wire);

private:
  std::map<ndn::Name, std::shared_ptr<ndn::Block>> m_contentWithBlock;
  mutable ndn::Block m_wire;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(State);

std::ostream&
operator<<(std::ostream& os, const State& state);

bool
operator==(const State& state1, const State& state2);

} // namespace psync

#endif // PSYNC_STATE_HPP
