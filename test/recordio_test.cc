#include <dmlc/io.h>
#include <dmlc/recordio.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: <filename>\n");
    return 0;
  }
  const unsigned kMagic = dmlc::RecordIOWriter::kMagic;
  unsigned bad[6] = {kMagic, 0, 2, 3, kMagic, kMagic};
  std::string data = "hello worldxx";
  {// output
    dmlc::Stream *fs = dmlc::Stream::Create(argv[1], "wb");
    dmlc::RecordIOWriter writer(fs);
    writer.WriteRecord(bad, sizeof(bad));
    writer.WriteRecord(data);
    delete fs;
    printf("finish writing with %lu exceptions\n", writer.except_counter());
  }
  {// input
    dmlc::Stream *fi = dmlc::Stream::Create(argv[1], "r");
    dmlc::RecordIOReader reader(fi);
    std::string temp;
    CHECK(reader.ReadRecord(&temp));
    CHECK(temp.length() == sizeof(bad));
    CHECK(!memcmp(temp.c_str(), bad, sizeof(bad)));
    CHECK(reader.ReadRecord(&temp));
    CHECK(temp == data);
    CHECK(!reader.ReadRecord(&temp));
    delete fi;
  }
  return 0;
}
