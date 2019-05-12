#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>

int indeks_queue = 0;
char data_queue[300][1000], front_queue[1000], larangan[1000]={"/home/adam/fp/lagu"}, gantikeroot[1000];

void push_queue(char masukan[1000]){
	indeks_queue++;
	strcpy(data_queue[indeks_queue - 1], masukan);
	
}
void pop_queue();

static const char *lokasi_asal = "/home/adam";

static int abc_getattr(const char *path, struct stat *stbuf){
	char fs[1000];
	sprintf(fs,"%s",path);

	char directory[1000];
	sprintf(directory,"%s%s",lokasi_asal,fs);
	
	int hasil;
	hasil = lstat(directory, stbuf);
	if (hasil == -1) return -errno;
	
	return 0;
}

static int abc_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi){
	char fs[1000];
	sprintf(fs,"%s",path);

	char directory[1000];
	sprintf(directory,"%s%s",lokasi_asal,fs);
	
	push_queue(directory);
	
	while(indeks_queue > 0)	{
		char lokasi_terkini[1000];
		DIR *berkas;
		struct dirent *file;
		(void) offset;
		
		struct stat efgh;
		
		(void) fi;

		pop_queue();
		strcpy(lokasi_terkini, front_queue);
		int panjang_lokasi_asal = strlen(lokasi_asal);

		if(strncmp(lokasi_terkini, "..", 3) == 0 && strncmp(lokasi_terkini, ".", 2) == 0) continue;
		if((lokasi_terkini[panjang_lokasi_asal + 2] == '.') && (lokasi_terkini[panjang_lokasi_asal + 1] == '.')) continue;
		
		if (opendir(lokasi_terkini) == NULL) return -errno;
		
		file = readdir(berkas);
		struct stat abcd;		
		
		while (file != NULL) {
			char nama_file[1000], isi_lokasi_terkini[1000];
			int proses = 0;	
				
			memset(&efgh, 0, sizeof(efgh));
			efgh.st_mode = file->d_type << 12;			
			
			memset(nama_file, 0, 1000);
			strcpy(nama_file, file->d_name);
			efgh.st_ino = file->d_ino;

			if(strncmp(nama_file, ".", 2) != 0 && strncmp(nama_file, "..", 3) != 0) {
				if((nama_file[0] != '.') && (nama_file[1] != '.') && strcmp(nama_file, "lagu") != 0){

					if(strncmp(lokasi_terkini, "/home/adam/", 12) != 0) sprintf(isi_lokasi_terkini, "%s/%s",lokasi_terkini, nama_file);
					else sprintf(isi_lokasi_terkini, "%s%s",lokasi_terkini, nama_file);

					stat(isi_lokasi_terkini, &abcd);

					if(S_ISREG(abcd.st_mode) == true) {
						int panjang_nama_file = strlen(nama_file);
						if(strcmp(nama_file + panjang_nama_file - 4,".mp3") == 0) proses = 0;
					
						if(proses != 0) continue;

						else if(proses == 0) {
							char filemp3[1000];
							memset(filemp3, 0, sizeof(filemp3));

							int indeksmp3 = 0, i = 0;
							while(i < strlen(isi_lokasi_terkini)){
								i++;
								if(lokasi_asal[i - 1] == isi_lokasi_terkini[i - 1]) {
									continue;
								}
								else{
									indeksmp3++;
									filemp3[indeksmp3 - 1] = isi_lokasi_terkini[i - 1];
								}		
							}
			
							char asal_root[30]={"/"};
							strcat(asal_root, nama_file);
							
							if(strcmp(filemp3, asal_root) == 0) proses = 1;

							memset(asal_root, 0, sizeof(asal_root));
								
							if(proses == 1) {
								if (filler(buf, nama_file, &efgh, 0));
								break;
							}
							else if(proses == 0) {
								memset(gantikeroot, 0, sizeof(gantikeroot));
								sprintf(gantikeroot,"%s/%s", lokasi_asal, nama_file);
								rename(isi_lokasi_terkini, gantikeroot);
							}		
						}		
					}
					else if(S_ISDIR(abcd.st_mode) == true) {
						int panjang_isi_lokasi_terkini = strlen(isi_lokasi_terkini);

						if((strncmp(nama_file, ".", 2) == 0 || strncmp(nama_file, "..", 3) == 0)) continue;

						else if((strncmp(nama_file, ".", 2) != 0 && strncmp(nama_file, "..", 3) != 0)) {

							if((isi_lokasi_terkini[panjang_isi_lokasi_terkini - 1] == '.') || (isi_lokasi_terkini[panjang_isi_lokasi_terkini - 2] == '.') || strncmp(nama_file, "lagu", 5) == 0) continue;

							else if((isi_lokasi_terkini[panjang_isi_lokasi_terkini - 1] != '.') && (isi_lokasi_terkini[panjang_isi_lokasi_terkini - 2] != '.') && strncmp(nama_file, "lagu", 5) != 0) push_queue(isi_lokasi_terkini);
						}
					}
				}					
			}		
		}
	closedir(berkas);
	}
	return 0;
}

static int abc_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi) {
	
	char fs[1000];
	sprintf(fs,"%s",path);

	char directory[1000];
	sprintf(directory,"%s%s",lokasi_asal,fs);
	
	int fd;
	(void) fi;
	fd = open(directory, O_RDONLY);
	if (fd == -1) return -errno;
	
	int hasil;
	hasil = pread(fd, buf, size, offset);
	if (hasil == -1) hasil = -errno;

	close(fd);
	return hasil;
}

static int abc_open(const char *path, struct fuse_file_info *fi)
{
	char fs[1000];
	sprintf(fs,"%s",path);
	
	char directory[1000];
	sprintf(directory,"%s%s",lokasi_asal,fs);

	int hasil;
	hasil = open(directory, fi->flags);
	if (hasil == -1) return -errno; 

	close(hasil);
	return 0;
}

static int abc_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi) {
	char fs[1000];
	sprintf(fs,"%s",path);

	char directory[1000];
	sprintf(directory,s"%s%s",lokasi_asal,fs);

	int fd;
	(void) path;
	fd=open(directory, O_WRONLY);
	if (fd == -1) return -errno;
	
	int hasil;
	hasil = pwrite(fd, buf, size, offset);
	if (hasil == -1) hasil = -errno;

	close(fd);
	return hasil;
}

static int abc_mknod(const char *path, mode_t mode, dev_t rdev) {
	char fs[1000];
	sprintf(fs,"%s",path);

	char directory[1000];
	sprintf(directory,"%s%s",lokasi_asal,fs);

	int hasil;
	if (S_ISREG(mode)) {
		hasil = open(directory, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (hasil >= 0)
			hasil = close(hasil);
	} 
	else if (S_ISFIFO(mode)) hasil = mkfifo(directory, mode);
	else hasil = mknod(directory, mode, rdev);
		
	if (hasil == -1) return -errno;
	return 0;
}

static int abc_utimeandstart(const char *path, const struct timespec ts[2]) {
	char fs[1000];
	sprintf(fs,"%s",path);
	
	char directory[1000];
	sprintf(directory, "%s%s",lokasi_asal,fs);

	struct timeval timevalue[2];
	timevalue[0].tv_sec = ts[0].tv_sec;
	timevalue[0].tv_usec = ts[0].tv_nsec / 1000;
	timevalue[1].tv_sec = ts[1].tv_sec;
	timevalue[1].tv_usec = ts[1].tv_nsec / 1000;

	int hasil;
	hasil = utimes(directory, timevalue);
	if (hasil == -1) return -errno;

	return 0;
}

static int abc_chown(const char *path, uid_t uid, gid_t gid) {
	char fs[1000];
	sprintf(fs,"%s",path);
	
	char directory[1000];
	sprintf(directory,"%s%s",lokasi_asal,fs);

	int hasil;

	hasil = lchown(directory, uid, gid);
	if (hasil == -1) return -errno;
	
	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= abc_getattr,
	.readdir	= abc_readdir,
	.utimens	= abc_utimeandstart,
	.read		= abc_read,
	.write		= abc_write,
	.chown		= abc_chown,
	.open		= abc_open,
	.mknod		= abc_mknod,
};

int main(int argc, char *argv[]) {
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}

void pop_queue(){
	strcpy(front_queue, data_queue[0]);
	for (int i = 0; i < indeks_queue; i++)
	{
		memset(data_queue[i], 0, sizeof(data_queue[i]));
		if(indeks_queue != 1) strcpy(data_queue[i], data_queue[i+1]);
	}
	indeks_queue--;
}
