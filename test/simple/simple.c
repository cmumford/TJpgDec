#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <tjpgd.h>

#define MODE                                                                   \
  0             /* Test mode: 0:JPEG to BMP conversion, 1:Output decode status \
                 */
#define SCALE 0 /* Output scaling 0:1/1, 1:1/2, 2:1/4 or 3:1/8 */

#if !defined(MAX_PATH)
#define MAX_PATH 256
#endif

/* User defined session identifier */

typedef struct {
  FILE* hin;       /* Handle to input stream */
  uint8_t* frmbuf; /* Pointer to the frame buffer */
  uint32_t wbyte;  /* Number of bytes a line of the frame buffer */
} IODEV;

#define COMPRESSON_NONE 0
#define COMPRESSON_RLE8 1
#define COMPRESSON_RLE4 2
#define COMPRESSON_RGB 3

typedef struct {
  uint16_t type; /* Magic identifier            */
  uint32_t size; /* File size in bytes          */
  uint16_t reserved1, reserved2;
  uint32_t offset; /* Offset to image data, bytes */
} BMP_FILEHEADER;

typedef struct {
  uint32_t size;                    /* Header size in bytes      */
  int32_t width, height;            /* Width and height of image */
  uint16_t planes;                  /* Number of colour planes   */
  uint16_t bits;                    /* Bits per pixel            */
  uint32_t compression;             /* Compression type          */
  uint32_t imagesize;               /* Image size in bytes       */
  int32_t xresolution, yresolution; /* Pixels per meter          */
  uint32_t ncolours;                /* Number of colours         */
  uint32_t importantcolours;        /* Important colours         */
} BMP_INFOHEADER;

/* User defined input function */

unsigned int
input_func(               /* Returns number of bytes read (zero on error) */
           JDEC* jd,      /* Decompression object */
           uint8_t* buff, /* Pointer to the read buffer (null to remove data) */
           unsigned int ndata /* Number of bytes to read/skip */
) {
  IODEV* dev = (IODEV*)jd->device;

  if (buff) { /* Read bytes */
    return (unsigned int)fread(buff, 1, ndata, dev->hin);
  } else { /* Remove bytes */
    int pos = fseek(dev->hin, ndata, SEEK_CUR);
    return pos < 0 ? 0 : ndata;
  }
}

/* User defined output function */

int output_func(              /* 1:Ok, 0:Aborted */
                JDEC* jd,     /* Decompression object */
                void* bitmap, /* Bitmap data to be output */
                JRECT* rect   /* Rectangular region to output */
) {
  uint32_t ny, nbx, xc, wd;
  uint8_t *src, *dst;
  IODEV* dev = (IODEV*)jd->device;

  /* Put progress indicator */
  if (MODE == 0 && rect->left == 0) {
    printf("\r%lu%%", (rect->top << jd->scale) * 100UL / jd->height);
  }

  nbx = (rect->right - rect->left + 1) *
        3; /* Number of bytes a line of the rectangular */
  ny = rect->bottom - rect->top + 1; /* Number of lines of the rectangular */
  src = (uint8_t*)bitmap;            /* RGB bitmap to be output */

  wd = dev->wbyte; /* Number of bytes a line of the frame buffer */
  dst = dev->frmbuf + rect->top * wd +
        rect->left *
            3; /* Left-top of the destination rectangular in the frame buffer */

  do { /* Copy the rectangular to the frame buffer */
    xc = nbx;
    do {
      *dst++ = *src++;
    } while (--xc);
    dst += wd - nbx;
  } while (--ny);

  return 1; /* Continue to decompress */
}

int write_bmp(const char* fname,
              uint8_t* buf,
              uint32_t width,
              uint32_t height) {
  uint32_t i, xb = (width * 3 + 3) & ~3;
  uint8_t *s, *d, r, g, b;
  FILE* hbmp;
  size_t written;

  hbmp = fopen(fname, "w");
  if (!hbmp)
    return errno;

  BMP_FILEHEADER bfh;
  memset(&bfh, 0, sizeof bfh);
  bfh.type = 'MB';
  bfh.size = sizeof(BMP_FILEHEADER) + sizeof(BMP_INFOHEADER) + xb * height;
  bfh.offset = sizeof(BMP_FILEHEADER) + sizeof(BMP_INFOHEADER);
  written = fwrite(&bfh, 1, sizeof(bfh), hbmp);
  if (written != sizeof(bfh)) {
    int err = errno;
    fclose(hbmp);
    return err;
  }

  BMP_INFOHEADER bih;
  memset(&bih, 0, sizeof bih);
  bih.size = sizeof bih;
  bih.width = width;
  bih.height = height;
  bih.planes = 1;
  bih.bits = 24;
  bih.compression = COMPRESSON_RGB;
  written = fwrite(&bih, 1, sizeof(bih), hbmp);
  if (written != sizeof(bih)) {
    int err = errno;
    fclose(hbmp);
    return err;
  }

  /* Flip top and down, swap byte order RGB to BGR */
  s = buf;
  d = buf + xb * (height - 1);
  while (s <= d) {
    for (i = 0; i < width * 3; i += 3) {
      r = s[i + 0];
      g = s[i + 1];
      b = s[i + 2];
      s[i + 0] = d[i + 2];
      s[i + 1] = d[i + 1];
      s[i + 2] = d[i + 0];
      d[i + 0] = b;
      d[i + 1] = g;
      d[i + 2] = r;
    }
    d -= xb;
    s += xb;
  }

  int err = 0;
  size_t to_write = height * xb;
  written = fwrite(buf, 1, to_write, hbmp);
  if (written != to_write)
    err = errno;

  fclose(hbmp);

  return err;
}

int jpegtest(char* fn) {
  const size_t sz_work = 4096; /* Size of working buffer for TJpgDec module */
  void* jdwork;                /* Pointer to TJpgDec work area */
  JDEC jd;                     /* TJpgDec decompression object */
  IODEV iodev; /* Identifier of the decompression session (depends on
                  application) */
  JRESULT rc;
  uint32_t xb, xs, ys;
  char str[256];

  printf("%s", fn); /* Put file name */

  /* Open JPEG file */
  iodev.hin = fopen(fn, "r");
  if (!iodev.hin)
    return errno;

  jdwork = calloc(1, sz_work);
  if (!jdwork) {
    fclose(iodev.hin);
    return ENOMEM;
  }

  rc = jd_prepare(&jd, input_func, jdwork, (unsigned int)sz_work,
                  &iodev); /* Prepare to decompress the file */

  if (!rc) {
    if (MODE == 1) {                         /* Put file properties */
      printf(",%u,%u", jd.width, jd.height); /* Image size */
      printf(",[%d:%d:%d]", 4, 4 / jd.msx,
             (jd.msy == 2) ? 0 : (jd.msx == 1) ? 4 : 2); /* Sampling ratio */
      printf(",%lu",
             sz_work -
                 jd.sz_pool); /* Get used memory size by rest of memory pool */
    } else {
      printf("\n");
    }
    xs = jd.width >> SCALE; /* Output size */
    ys = jd.height >> SCALE;
    xb = (xs * 3 + 3) & ~3; /* Byte width of the frame buffer */
    iodev.wbyte = xb;
    iodev.frmbuf = malloc(xb * ys); /* Allocate an output frame buffer */
    rc = jd_decomp(&jd, output_func, SCALE); /* Start to decompress */
    if (!rc) { /* Save the decompressed picture as a bmp file */
      if (MODE == 1) {
        printf(",%d", rc);
      } else {
        printf("\rOK  ");
        strcpy(str, fn);
        strcpy(str + strlen(str) - 4, ".bmp");
        rc = write_bmp(str, (uint8_t*)iodev.frmbuf, xs, ys);
      }
    } else { /* Error occured on decompress */
      if (MODE == 1) {
        printf(",%d", rc);
      } else {
        printf("\rError(%d)", rc);
      }
    }
    free(iodev.frmbuf); /* Discard frame buffer */
  } else {              /* Error occured on prepare */
    if (MODE == 1) {
      printf(",,,,,%d", rc);
    } else {
      printf("\nErr: %d", rc);
    }
  }

  free(jdwork); /* Discard work area */

  fclose(iodev.hin); /* Close JPEG file */

  printf("\n");
  return 0;
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
      return "";
    return dot;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "\n\nusage:\n\n");
    fprintf(stderr, "simple <image_dir>\n\n");
    return ENOENT;
  }

  if (MODE == 1)
    printf("File Name,Width,Height,Sampling,Used Memory,Result\n");

  DIR* d = opendir(argv[1]);
  if (!d) {
    fprintf(stderr, "Cannot open \"%s\"\n", argv[1]);
    return errno;
  }

  struct dirent* dir;
  while ((dir = readdir(d))) {
    if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
        continue;
    const char* extn = get_filename_ext(dir->d_name);
    if (!strcasecmp(extn, ".jpg") || !strcasecmp(extn, ".jpeg")) {
      char file_name[MAX_PATH];
      if (snprintf(file_name, MAX_PATH, "%s/%s", argv[1], dir->d_name) > 0) {
        fprintf(stdout, "Opening \"%s\"\n", file_name);
        jpegtest(file_name);
      }
    }
  }

  closedir(d);

  return 0;
}
