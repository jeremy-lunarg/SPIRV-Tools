// Copyright (c) 2025 LunarG Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <vector>
#include <unordered_map>

#include "source/opt/pass.h"

namespace spvtools {
namespace opt {

class RemapIdsPass : public Pass {
  public:
    RemapIdsPass() = default;
    virtual ~RemapIdsPass() = default;

    Pass::Status Process() override;

    const char* name() const override { return "remap"; }
  private:
    // TODO: For all code, verify that variables and functions are named in accordance with
    // the Google style guide.

    // special values for ids
    static constexpr spv::Id unmapped_ {spv::Id(-10000)};
    static constexpr spv::Id unused_ {spv::Id(-10001)};

    std::vector<spv::Id> type_and_const_ids_;
    std::unordered_map<std::string, spv::Id> name_ids_;
    std::vector<spv::Id> function_ids_;
    std::vector<spv::Id> remainder_ids;

    void ScanIds();
    void RemapTypeAndConst();
    spv::Id HashTypeAndConst(spv::Id id) const;
    void RemapNames();
    void RemapFunctions();
    spv::Id HashOpCode(Instruction* inst) const;
    void RemapRemainders();
    void ApplyMap();
    void ReplaceId(spv::Id old_id, spv::Id new_id);
    bool IsTypeOp(spv::Op opCode) const;
    bool IsConstOp(spv::Op opCode) const;
    spv::Id GetBound() const; // all <id>s are guaranteed to satisfy 0 < id < bound.
    void UpdateBound();
    inline spv::Id NextUnusedId(spv::Id id); // return next unused new id.

    std::string IdAsString(spv::Id const id) const;
    void PrintId(spv::Id const id) const;
    void PrintNewIds() const;


    // TODO: JJH - wouldn't a simple unordered_set suffice?
    // implement a simple dynamic bitset to track which new IDs have been
    // allocated rather than searching the spare new id space.
    typedef std::uint64_t bits_t;
    std::vector<bits_t> mapped_; // which new IDs have been mapped
    // TODO: JJH - Why are half the bits used? The 4 should be an 8.
    static const int m_bits_ = sizeof(bits_t) * 4; // modulo bits
    bool IsMapped(spv::Id id) const  {
      return id < MaxMappedId() && ((mapped_[id/m_bits_] & (1LL<<(id%m_bits_))) != 0);
    }
    void SetMapped(spv::Id id) { ResizeMapped(id); mapped_[id/m_bits_] |= (1LL<<(id%m_bits_)); }
    void ResizeMapped(spv::Id id) { if (id >= MaxMappedId()) mapped_.resize(id/m_bits_+1, 0); }
    size_t MaxMappedId() const { return mapped_.size() * m_bits_; }
    bool IsNewIdMapped(spv::Id new_id) const { return IsMapped(new_id);            }
    bool IsOldIdUnmapped(spv::Id old_id) const { return GetNewId(old_id) == unmapped_; }
    bool IsOldIdUnused(spv::Id old_id) const { return GetNewId(old_id) == unused_;   }
    bool IsOldIdMapped(spv::Id oldId) const { return !IsOldIdUnused(oldId) && !IsOldIdUnmapped(oldId); }

    // a mapping from old ids to new ids e.g. new_id_[old_id] = new_id
    std::vector<spv::Id> new_id_;
    spv::Id GetNewId(spv::Id id) const { return new_id_[id]; }
    void SetNewId(spv::Id id, spv::Id new_id = unmapped_);




    // TODO: clean up leftover cruft from naive port attempt
    void BuildLocalMaps();
    void ProcessInstruction(Instruction* inst);

    // TODO: Remove idfn_t and instfn_t typedefs.
    typedef std::function<void(spv::Id)> idfn_t;
    typedef std::function<bool(spv::Op)> instfn_t;

    // Map of names to IDs
    typedef std::unordered_map<std::string, spv::Id> namemap_t;
    namemap_t name_map_; // ID names from OpName
 
    // Function start and end.  use unordered_map because we'll have
    // many fewer functions than IDs.
    typedef std::pair<unsigned, unsigned> range_t;
    std::unordered_map<spv::Id, range_t> fn_pos_;
    // Which functions are called, anywhere in the module, with a call count
    std::unordered_map<spv::Id, int> fn_calls_;

       // A set that preserves position order, and a reverse map
    typedef std::set<int> posmap_t;
    posmap_t type_const_pos_; // word positions that define types & consts (ordered)
    typedef std::unordered_map<spv::Id, int> posmap_rev_t;
    posmap_rev_t id_pos_r_; // reverse map from IDs to positions

};

} // namespace opt
} // namespace spvtools
