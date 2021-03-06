#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int CheckFile(string filename, const size_t chunk_size) {
  struct stat file_stat;
  if (stat(filename.c_str(), &file_stat) == -1) {
    cerr << filename << ": " << strerror(errno) << endl;
    return -1;
  }
  const auto file_size = file_stat.st_size;
  const auto block_size = file_stat.st_blksize;
  // cerr << "file size: " << file_size << ", block size: " << block_size
  //      << ", chunk size: " << chunk_size << endl;

  int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) {
    cerr << filename << ": " << strerror(errno) << endl;
    return -1;
  }

  int64_t start = -1;
  int64_t end = -1;
  vector<uint8_t> buf(block_size);

  auto read_block = [&buf, &filename, fd, block_size, file_size,
                     chunk_size](off_t read_offset) -> ssize_t {
    assert(read_offset >= 0);
    assert(read_offset < file_size);
    ssize_t read_size = min(block_size, file_size - read_offset);
    // cerr << "reading\t" << read_size << "\tat\t" << read_offset << "\tfrom\t"
    //      << filename << endl;
    auto res = pread(fd, &buf[0], read_size, read_offset);
    if (res == -1) {
      cerr << filename << ": " << strerror(errno) << endl;
      return -1;
    } else if (res != read_size) {
      cerr << filename << ": "
           << "short read" << endl;
      return -1;
    }
    return res;
  };

  auto check_read = [&buf](ssize_t read_size) -> bool {
    for (int i = 0; i < read_size; ++i) {
      if (buf[i]) {
        return false;
      }
    }
    return true;
  };

  auto update_interval = [&start, &end, &filename, &file_size](
      off_t offset, ssize_t read_size, bool all_zeros) -> bool {
    if (all_zeros) {
      if (start == -1) {
        start = offset;
      }
      end = offset + read_size;
    }
    if ((!all_zeros && start != -1) || end == file_size) {
      cout << filename << "\t" << start << "\t" << end - start << endl;
      start = -1;
    }
    return all_zeros;
  };

  for (off_t offset = 0; offset < file_size;) {
    auto read_size = read_block(offset);
    if (read_size == -1) {
      return -1;
    }
    const bool all_zeros = check_read(read_size);
    update_interval(offset, read_size, all_zeros);
    offset += read_size;
    if (all_zeros || !(offset % chunk_size)) {
      continue;
    } else {
      offset = ((offset / chunk_size) + 1) * chunk_size;
    }
  }
  close(fd);
  return 0;
}

int main(int argc, char **argv) {
  size_t chunk_size = 64 * 1024 * 1024;
  int opt;
  while ((opt = getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
      case 'c':
        chunk_size = atoi(optarg);
        break;
      default:
        cerr << "invalid option: " << opt << endl;
        return 1;
    }
  }
  if (optind >= argc) {
    cerr << "must provide filename" << endl;
    return 1;
  }
  const string filename = argv[optind];

  return CheckFile(filename, chunk_size);
}
