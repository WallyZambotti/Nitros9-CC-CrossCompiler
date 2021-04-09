#if 0
typedef enum
{
	object_kind_os9,
	object_kind_decb

}               object_type;
#endif

#if 0
struct os9
{
	unsigned short  module_size;
	unsigned short  offset_to_module_name;
	int             type_language;
	int             attr_rev;
	int             execution_offset;
	unsigned short  permanent_storage_size;
	char            module_name[29 + 1];
	int             edition;
};

struct decb
{
	uint16_t        segment_size;
	uint16_t        org_offset;
};
#endif

struct object_header
{
#ifdef COCO
	unsigned   module_size;
	unsigned   offset_to_module_name;
#else
	unsigned short  module_size;
	unsigned short  offset_to_module_name;
#endif
	int             type_language;
	int             attr_rev;
	int             execution_offset;
#ifdef COCO
	unsigned   permanent_storage_size;
#else
	unsigned short  permanent_storage_size;
#endif
	char            module_name[29 + 1];
	int             edition;

#if 0
object_type     kind;
	union
	{
		struct os9      os9;
		struct decb     decb;
	};
#endif
};

int     os9_hdr();
int     os9_bdyb();
int     os9_body();
int     os9_tail();

int     dcb_hdr();
int     dcb_bdyb();
int     dcb_body();
int     dcb_tail();
