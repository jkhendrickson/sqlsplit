/*
 * sqlsplitter.c
 * by: Jeff Hendrickson
 * Date                                         Version
 *--------------------                          ---------------------
 * November 14, 2019                            Initial Revision 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int writeSchema(char* schema, int count);
int readThruData(FILE* fp, int linelen);
 
int main(int argc, char *argv[])
{
	FILE *fp;
	// prove me wrong
	bool bSeparate = true;
	if (argc < 2) {
		printf("sample use:\n");
		printf("%s (separate:single)\n",argv[0]);
		printf("%s separate - each table gets own file e.g. sqlTable1.sql, sqlTable2.sql\n",argv[0]);
		printf("%s single - all tables go in one file e.g. sqlTable0.sql\n",argv[0]);
		exit(-1);
	}
	if(strcasecmp(argv[1],"single")==0) 
		bSeparate = false;	
 	// JKH add your huuuuge .sql dump filename here
 	// I tested this on a 52GB file!
	fp = fopen("20190924-042849.sql", "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);
	
	char cbBuffer[40960];
	char buf[1024], *p;
	int lineno = 0;
	int started, ended;
	started = ended = 0;
	while (!feof(fp)) {
		if (fgets(buf,sizeof(buf),fp) != NULL) {
			if ((p = strchr(buf, '\n')) == NULL) {
				// printf("input line too long.\n");
				continue;
			}
			char cbOut[128];
			if(strncmp(buf,"CREATE TABLE",strlen("CREATE TABLE"))==0) {		
				strncpy(cbOut,buf,strlen(buf)-1);
				cbOut[strlen(buf)-1] = 0;
				if(bSeparate) 
					++lineno;
				printf("%d: %s\n",lineno,cbOut);
				started = 1;
			} else if (strncmp(buf,"LOCK TABLES",strlen("LOCK TABLES"))==0) {
				strncpy(cbOut,buf,strlen(buf)-1);
				cbOut[strlen(buf)-1] = 0;
				printf("%d: %s\n",lineno,cbOut);
				ended = 1;			
			}
			if(started==1) {
				if(ended==1) {
					started = ended = 0;
					writeSchema(cbBuffer, lineno);
					memset(cbBuffer,0x0,sizeof(cbBuffer));
				} else {
					strcat(cbBuffer,buf);
				}
			}
		}	
	}		
	
	fclose(fp);
	exit(EXIT_SUCCESS);
}

int readThruData(FILE* fp, int linelen) {
	int retcode = 0;
	int count = 0;
	int c = 0;
	// this function supported proof of concept...
	// read through the file until we hit \n, or EOF
	while (count++ < linelen) {
        if ((c = getc(fp)) == EOF || c == '\n') {
            retcode = 1;
            break;
        }
    }
	return retcode;
}

int writeSchema(char* schema, int count) {
	int retcode = 1;
	char cbFilename[64];
	sprintf(cbFilename,"sqlTable%d.sql",count);
	int len = strlen(schema);
	// append to the file...
	FILE *f_dst = fopen(cbFilename, "ab");
	if (f_dst == NULL) {
		printf("ERROR - Failed to open file for writing\n");
		retcode = 0;
	}

	if (fwrite(schema, sizeof(char), len, f_dst) != len) {
		printf("ERROR - Failed to write %i bytes to file\n", len);
		retcode = 0;
	} else {
		printf("Wrote %i bytes to file\n", len);
	}

	fclose(f_dst);
	f_dst = NULL;
	
	return retcode;
}