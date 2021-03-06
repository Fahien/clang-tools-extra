//===--- Serialization.h - Binary serialization of index data ----*- C++-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides serialization of indexed symbols and other data.
//
// It writes sections:
//  - metadata such as version info
//  - a string table (which is compressed)
//  - lists of encoded symbols
//
// The format has a simple versioning scheme: the format version number is
// written in the file and non-current versions are rejected when reading.
//
// Human-readable YAML serialization is also supported, and recommended for
// debugging and experiments only.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANGD_INDEX_RIFF_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANGD_INDEX_RIFF_H
#include "Index.h"
#include "llvm/Support/Error.h"

namespace clang {
namespace clangd {

enum class IndexFileFormat {
  RIFF, // Versioned binary format, suitable for production use.
  YAML, // Human-readable format, suitable for experiments and debugging.
};

// Holds the contents of an index file that was read.
struct IndexFileIn {
  using FileDigest = std::array<uint8_t, 20>;
  llvm::Optional<SymbolSlab> Symbols;
  llvm::Optional<RefSlab> Refs;
  // Digest of the source file that generated the contents.
  llvm::Optional<FileDigest> Digest;
};
// Parse an index file. The input must be a RIFF or YAML file.
llvm::Expected<IndexFileIn> readIndexFile(llvm::StringRef);

// Specifies the contents of an index file to be written.
struct IndexFileOut {
  const SymbolSlab *Symbols = nullptr;
  const RefSlab *Refs = nullptr;
  // Digest of the source file that generated the contents.
  const IndexFileIn::FileDigest *Digest = nullptr;
  // TODO: Support serializing Dex posting lists.
  IndexFileFormat Format = IndexFileFormat::RIFF;

  IndexFileOut() = default;
  IndexFileOut(const IndexFileIn &I)
      : Symbols(I.Symbols ? I.Symbols.getPointer() : nullptr),
        Refs(I.Refs ? I.Refs.getPointer() : nullptr) {}
};
// Serializes an index file.
llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const IndexFileOut &O);

// Convert a single symbol to YAML, a nice debug representation.
std::string toYAML(const Symbol &);
std::string toYAML(const std::pair<SymbolID, ArrayRef<Ref>> &);

// Build an in-memory static index from an index file.
// The size should be relatively small, so data can be managed in memory.
std::unique_ptr<SymbolIndex> loadIndex(llvm::StringRef Filename,
                                       llvm::ArrayRef<std::string> URISchemes,
                                       bool UseDex = true);

} // namespace clangd
} // namespace clang

#endif
