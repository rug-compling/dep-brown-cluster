#include "strdb.h"
#include "str.h"


void destroy_strings(StrVec &vec) {
  foridx(i, len(vec))
    delete [] vec[i];
}

void destroy_strings(StrStrMap &map) {
  typedef const char *const_char_ptr;
  StrVec strs;
  formap(const_char_ptr, s, const_char_ptr, t, StrStrMap, map) {
    strs.push_back(s);
    strs.push_back(t);
  }
  destroy_strings(strs);
}

////////////////////////////////////////////////////////////

int StrDB::read(istream &in, int N, bool one_way) {
  char s[16384];
  clear();
  while(in.getline(s, 16384)) { //dlm: space separated tokens on one line is 1 token
    if(one_way) i2s.push_back(copy_str(s));
    else (*this)[s];
  }
  logs(size() << " strings read");
  return size();
}

int StrDB::read(const char *file, bool one_way) {
  track("StrDB::read()", file << ", one_way=" << one_way, true);
  ifstream in(file);
  assert(in);
  return read(in, INT_MAX, one_way);
}

void StrDB::write(ostream &out) {
  foridx(i, size())
    out << i2s[i] << endl;
  logs(size() << " strings written");
}

void StrDB::write(const char *file) {
  track("StrDB::write()", file, true);
  ofstream out(file);
  write(out);
}

const char *StrDB::operator[](int i) const {
  assert(i >= 0 && i < len(i2s));
  return i2s[i];
}

int StrDB::lookup(const char *s, bool incorp_new, int default_i) {
  StrIntMap::const_iterator it = s2i.find(s);
  if(it != s2i.end()) return it->second;
  if(incorp_new) {
    char *t = copy_str(s);
    int i = s2i[t] = len(i2s);
    i2s.push_back(t);
    return i;
  }
  else
    return default_i;
}

IntVec StrDB::lookup(const StrVec &svec) {
  IntVec ivec(len(svec));
  foridx(i, len(svec))
    ivec[i] = lookup(svec[i], true, -1);
  return ivec;
}

int StrDB::operator[](const char *s) const {
  StrIntMap::const_iterator it = s2i.find(s);
  if(it != s2i.end()) return it->second;
  return -1;
}

int StrDB::operator[](const char *s) {
  return lookup(s, true, -1);
}

ostream &operator<<(ostream &out, const StrDB &db) {
  foridx(i, len(db)) out << db[i] << endl;
  return out;
}

////////////////////////////////////////////////////////////

int IntPairIntDB::lookup(const IntPair &p, bool incorp_new, int default_i) {
  IntPairIntMap::const_iterator it = p2i.find(p);
  if(it != p2i.end()) return it->second;

  if(incorp_new) {
    int i = p2i[p] = len(i2p);
    i2p.push_back(p);
    return i;
  }
  else
    return default_i;
}

int IntPairIntDB::read(istream &in, int N) {
  assert(size() == 0);
  int a, b;
  while(size() < N && in >> a >> b)
    (*this)[IntPair(a, b)];
  return size();
}

void IntPairIntDB::write(ostream &out) {
  forvec(_, const IntPair &, p, i2p)
    out << p.first << ' ' << p.second << endl;
}

////////////////////////////////////////////////////////////

int IntVecIntDB::lookup(const IntVec &v, bool incorp_new, int default_i) {
  IntVecIntMap::const_iterator it = v2i.find(v);
  if(it != v2i.end()) return it->second;

  if(incorp_new) {
    int i = v2i[v] = len(i2v);
    i2v.push_back(v);
    return i;
  }
  else
    return default_i;
}

////////////////////////////////////////////////////////////
//dlm:
// Read the lines from file, put head and dependent in db,
// and call back func with head, dep and count (builds text representation).
// But if the db already exists and the strings have been converted
// into integers (i.e., <file>.{strdb,int*} exist), then use those.
// If incorp_new is false, then words not in db will just get passed -1.

typedef void intvec_func(IntVec tuple);//dlm: IntVec instead of int

void read_text(const char *file, intvec_func *func, StrDB &db, bool read_cached, bool write_cached, bool incorp_new) {//dlm: intvec_func
  track("read_text()", file, true);

  string strdb_file = string(file)+".strdb";
  string int_file_h = string(file)+".int_h"; //dlm: int file is now three files, for heads, deps and counts
  string int_file_d = string(file)+".int_d"; //dlm
  string int_file_c = string(file)+".int_c"; //dlm

  // Use the cached strdb and intfiles only if they exist and they are
  // newer than the text file.
  read_cached &= file_exists(strdb_file.c_str()) && 
                 file_exists(int_file_h.c_str()) &&
                 file_exists(int_file_d.c_str()) &&
                 file_exists(int_file_c.c_str()) &&
                 file_modified_time(strdb_file.c_str()) > file_modified_time(file) &&
                 file_modified_time(int_file_h.c_str()) > file_modified_time(file) &&
                 file_modified_time(int_file_d.c_str()) > file_modified_time(file) &&
                 file_modified_time(int_file_c.c_str()) > file_modified_time(file); //dlm: extra conditions for int files

  if(read_cached) {
    //dlm: Read from strdb and int files.
    assert(db.size() == 0); // db must be empty because we're going to clobber it all
    db.read(strdb_file.c_str(), true);
    track_block("", "Reading from " << int_file_h << "," << int_file_d << "," << int_file_c, false) { //dlm: extra int files
      ifstream in_h(int_file_h.c_str()); //dlm
      ifstream in_d(int_file_d.c_str()); //dlm
      ifstream in_c(int_file_c.c_str()); //dlm
      char buf_h[16384];//dlm: now three buffers
      char buf_d[16384];//dlm
      char buf_c[16384];//dlm
      while(true) {
        in_h.read(buf_h, sizeof(buf_h)); //dlm
        in_d.read(buf_d, sizeof(buf_d)); //dlm
        in_c.read(buf_c, sizeof(buf_c)); //dlm
        if(in_h.gcount() == 0) break; //dlm
        if(in_d.gcount() == 0) break; //dlm
        if(in_c.gcount() == 0) break; //dlm

        assert(in_h.gcount() % sizeof(int) == 0); //dlm
        assert(in_d.gcount() % sizeof(int) == 0); //dlm
        assert(in_c.gcount() % sizeof(int) == 0); //dlm
	
	int buf_i_d = 0; //dlm
	int buf_i_c = 0; //dlm
        for(int buf_i_h = 0; buf_i_h < in_h.gcount(); buf_i_h += 4) {//dlm iterate over buf_i_h
	  IntVec tuple; //dlm: tuple to be created in this iter
          int a = *((int *)(buf_h+buf_i_h)); //dlm
	  assert(a >= 0 && a < db.size()); //dlm
	  tuple.push_back(a); //dlm

	  if (buf_i_d < in_d.gcount()) { //dlm: do the dep
	    int b = *((int *)(buf_d+buf_i_d)); //dlm
	    assert(b >= 0 && b < db.size()); //dlm
	    buf_i_d += 4; //dlm
	    tuple.push_back(b); //dlm
	  }
	  
	  if (buf_i_c < in_c.gcount()) { //dlm: do the count
	    int count = *((int *)(buf_c+buf_i_c)); //dlm
	    buf_i_c += 4; //dlm
	    tuple.push_back(count); //dlm
	  }
	  
	  func(tuple); //dlm: tuple is passed to func instead of int a
	}
      }
    }
  }
  else {
    track_block("", "Reading from " << file, false) {
      //dlm: Write to strdb and int files. 
      ifstream in(file);

      ofstream out_h;//dlm: heads
      ofstream out_d;//dlm: dependents
      ofstream out_c;//dlm: counts
      //#if 0 //dlm      
      if(write_cached) { //dlm: caching and writing in three steps
        out_h.open(int_file_h.c_str()); //dlm
        out_d.open(int_file_d.c_str()); //dlm
        out_c.open(int_file_c.c_str()); //dlm
        if(!(out_h && out_d && out_c)) write_cached = false; //dlm
      }
      if(write_cached) logs("Writing to " << int_file_h << "," << int_file_d << "," << int_file_c); //dlm: extra int files
      //#endif

      char line[16384]; //dlm
      char head[16384]; //dlm
      char dep[16384]; //dlm
      char ccount[16384]; //dlm: char-type count
      int count; //dlm

      char buf_h[16384]; int buf_i_h = 0; //dlm
      char buf_d[16384]; int buf_i_d = 0; //dlm
      char buf_c[16384]; int buf_i_c = 0; //dlm
            
      while(in.getline(line, 16384)){ //dlm: line is a tab-sep sequence of head, dep and count
	if (sscanf(line, "%[^\t]\t%[^\t]\t%[^\t]", head, dep, ccount) != 3) continue; //dlm
	count = atoi(ccount); //dlm
	
	IntVec tuple; //dlm
	
	int a = db.lookup(head, incorp_new, -1); //dlm
	int b = db.lookup(dep, incorp_new, -1); //dlm
	
	tuple.push_back(a); //dlm
	tuple.push_back(b); //dlm
	tuple.push_back(count); //dlm
	
        if(func) func(tuple); //dlm: tuple instead of int a
	//#if 0
        if(write_cached) {
	  //dlm: buffering heads (a)
          if(buf_i_h + sizeof(a) > sizeof(buf_h)) { //dlm
            out_h.write(buf_h, buf_i_h); //dlm
            buf_i_h = 0; //dlm
          }
          *((int *)(buf_h+buf_i_h)) = a; //dlm
          buf_i_h += sizeof(a); //dlm

	  //dlm: buffering deps (b)
	  if(buf_i_d + sizeof(b) > sizeof(buf_d)) { //dlm
            out_d.write(buf_d, buf_i_d); //dlm
            buf_i_d = 0; //dlm
          }
          *((int *)(buf_d+buf_i_d)) = b; //dlm
          buf_i_d += sizeof(b);//dlm

	  //dlm: buffering counts (count)
	  if(buf_i_c + sizeof(count) > sizeof(buf_c)) { //dlm
            out_c.write(buf_c, buf_i_c); //dlm
            buf_i_c = 0; //dlm
          }
          *((int *)(buf_c+buf_i_c)) = count; //dlm
          buf_i_c += sizeof(count); //dlm
	}
	//#endif
      }
      //#if 0
      if(write_cached) {// Final flush
        out_h.write(buf_h, buf_i_h); //dlm
	out_d.write(buf_d, buf_i_d); //dlm
	out_c.write(buf_c, buf_i_c); //dlm
      }
      //#endif
    }

    if(write_cached && create_file(strdb_file.c_str()))
      db.write(strdb_file.c_str());
  }
}
