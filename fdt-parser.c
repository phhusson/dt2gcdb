#include <libfdt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>

struct dsi_ctrl_hdr {                                                                                                                               
    char dtype; /* data type */
    char last;  /* last in chain */

    char vc;    /* virtual chan */
    char ack;   /* ask ACK from peripheral */

    char wait;  /* ms */
    uint16_t dlen; /* 16 bits */
} __attribute__ ((__packed__));

int indent = 0;
void print_indent() {
	for(int i=0; i<indent; ++i)
		printf("  ");
}

void dump_cmds(const struct fdt_property *prop, const char* panel, const char *variable_name, int p) {
	int v = 0;
	for(int pos=0; pos<fdt32_to_cpu(prop->len); ) {
		struct dsi_ctrl_hdr *hdr = (struct dsi_ctrl_hdr*)(((uint8_t*)prop->data)+pos);
		int len = ntohs(hdr->dlen);
		pos += sizeof(*hdr);

		if(hdr->vc != 0)
			exit(1);

		//DTYPE_DCS_LWRITE
		//DTYPE_GEN_LWRITE
		if(hdr->dtype == 0x39 || hdr->dtype == 0x29) { 
			//0x40 = long packet
			uint8_t val = 0x40;
			if(hdr->last)
				val |= 0x80;
			if(hdr->ack)
				val |= 0x20;
			printf("%01d10%03d static char %s_%s_cmd%d = { 0x%02x, 0x%02x, 0x%02x, 0x%02x, ", p, v, panel, variable_name, v,
					len, 0, hdr->dtype, val);

			int remaining = len % 4;
			if(remaining)
				remaining = 4 - remaining;

			for(int i=0; i< len-1; ++i)
				printf("0x%02hhx, ", prop->data[pos+i]);
			if(len) {
				if(!remaining)
					printf("0x%02hhx", prop->data[pos+len-1]);
				else
					printf("0x%02hhx, ", prop->data[pos+len-1]);
			}

			//Pad to 4
			for(int i=0; i<(remaining-1); ++i)
				printf("0xff, ");
			if(remaining)
				printf("0xff");

			printf("};\n");

			printf("%01d20%03d { 0x%02x, %s_%s_cmd%d, 0x%02x},\n", p, v, 4 + len + remaining, panel, variable_name, v, hdr->wait);
		//DTYPE_DCS_WRITE
		} else if(hdr->dtype == 0x05) {
			if(len != 2)
				exit(1);
			uint8_t val = 0;
			if(hdr->last)
				val |= 0x80;
			if(hdr->ack)
				val |= 0x20;
			printf("%01d10%03d static char %s_%s_cmd%d = { 0x%02x, 0x%02x, 0x%02x, 0x%02x};\n", p, v, panel, variable_name, v,
					prop->data[pos], 0, hdr->dtype, 0x80);
			printf("%01d20%03d { 0x%02x, %s_%s_cmd%d, 0x%02x},\n", p, v, 4, panel, variable_name, v, hdr->wait);
		} else {
			printf("==Unsupported %x\n", hdr->dtype);
			exit(1);
		}

		pos += len;
		++v;
	}
}

void dump_resolution(void *fdt, int id, const char *panel) {
	const struct fdt_property *tmp = NULL;
    uint16_t panel_width;
    uint16_t panel_height;
    uint16_t hfront_porch;
    uint16_t hback_porch;
    uint16_t hpulse_width;
    uint16_t hsync_skew;
    uint16_t vfront_porch;
    uint16_t vback_porch;
    uint16_t vpulse_width;
    uint16_t hleft_border;
    uint16_t hright_border;
    uint16_t vtop_border;
    uint16_t vbottom_border;
    uint16_t hactive_res;
    uint16_t vactive_res;
    uint16_t invert_data_polarity;
    uint16_t invert_vsync_polarity;
    uint16_t invert_hsync_polarity;

       
	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-panel-width", NULL);
	panel_width = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-panel-height", NULL);
	panel_height = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-h-front-porch", NULL);
	hfront_porch = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-h-back-porch", NULL);
	hback_porch = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-h-pulse-width", NULL);
	hpulse_width = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-h-sync-skew", NULL);
	hsync_skew = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-v-front-porch", NULL);
	vfront_porch = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-v-back-porch", NULL);
	vback_porch = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-v-pulse-width", NULL);
	vpulse_width = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-h-left-border", NULL);
	hleft_border = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-h-right-border", NULL);
	hright_border = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-v-top-border", NULL);
	vtop_border = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-v-bottom-border", NULL);
	vbottom_border = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-v-bottom-border", NULL);
	vbottom_border = fdt32_to_cpu( *(uint32_t*)tmp->data);

hactive_res = vactive_res = 0; // Unused in LK
invert_data_polarity = invert_vsync_polarity = invert_hsync_polarity = 0; //Unused in LK


	printf(
"static struct panel_resolution %s_panel_res = { \n"
"		    %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,\n"
"};\n",
	panel,
	panel_width, panel_height,
	hfront_porch, hback_porch, hpulse_width, hsync_skew,
	vfront_porch, vback_porch, vpulse_width,
	hleft_border, hright_border, vtop_border, vbottom_border,
	hactive_res, vactive_res, invert_data_polarity, invert_vsync_polarity, invert_hsync_polarity);
}

void dump_colorinfo(void *fdt, int id, const char *panel) {
	const struct fdt_property *tmp = NULL;
    uint8_t  color_format;
    uint8_t  color_order;
    uint8_t  underflow_color;
    uint8_t  border_color;
    uint8_t  pixel_packing;
    uint8_t  pixel_alignment;

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-bpp", NULL);
	color_format = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-color-order", NULL);
	color_order = 0; //TBD (a string)

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-underflow-color", NULL);
	underflow_color = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-border-color", NULL);
	border_color = fdt32_to_cpu( *(uint32_t*)tmp->data);

pixel_packing = pixel_alignment = 0; // pixel_alignment not used, pixel_packing used, not found

printf("static struct color_info %s_color = {\n"
"    %d, %d, 0x%02x, %d, %d, %d\n"
"};\n",
	panel,
	color_format, color_order,
	underflow_color, border_color, pixel_packing, pixel_alignment);
}

void dump_state(void *fdt, int id, const char *panel) {
	const struct fdt_property *tmp = NULL;
uint8_t onstate;
uint8_t offstate;

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-on-command-state", NULL);
	if(strcmp(tmp->data, "dsi_lp_mode") == 0)
		onstate = 0;
	else if(strcmp(tmp->data, "dsi_hs_mode") == 0)
		offstate = 1;
	else
		exit(1);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-off-command-state", NULL);
	if(strcmp(tmp->data, "dsi_lp_mode") == 0)
		offstate = 0;
	else if(strcmp(tmp->data, "dsi_hs_mode") == 0)
		offstate = 1;
	else
		exit(1);

printf("static struct command_state %s_state = {\n"
"    %d, %d\n"
"};\n", panel, onstate, offstate);
}

void dump_misc(void *fdt, int id, const char *panel) {
printf("static struct commandpanel_info %s_command_panel = {\n"
"    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0\n"
"};\n", panel);
}

void dump_panelinfo(void *fdt, int id, const char *panel) {
	const struct fdt_property *tmp = NULL;
    uint8_t hsync_pulse;
    uint8_t hfp_power_mode;
    uint8_t hbp_power_mode;
    uint8_t hsa_power_mode;
    uint8_t bllp_eof_power_mode;
    uint8_t bllp_power_mode;
    uint8_t traffic_mode;
    uint8_t dma_delayafter_vsync;
    uint32_t  bllp_eof_power;

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-traffic-mode", NULL);
	if(strcmp(tmp->data, "burst_mode") == 0)
		traffic_mode = 2;
	else if(strcmp(tmp->data, "non_burst_sync_event") == 0) 
		traffic_mode = 1;
	else
		exit(1);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-h-sync-pulse", NULL);
	hsync_pulse = fdt32_to_cpu( *(uint32_t*)tmp->data);

printf("static struct videopanel_info %s_video_panel = {\n"
"    %d, 0, 0, 0, 1, 1, %d, 0, 0x9\n"
"};\n", panel,
	hsync_pulse, traffic_mode);
}

void dump_lane(void *fdt, int id, const char *panel) {
	const struct fdt_property *tmp = NULL;

    uint8_t dsi_lanes;
    uint8_t dsi_lanemap;
    uint8_t lane0_state = 0;
    uint8_t lane1_state = 0;
    uint8_t lane2_state = 0;
    uint8_t lane3_state = 0;

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-lane-0-state", NULL);
	if(tmp) lane0_state = 1;

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-lane-1-state", NULL);
	if(tmp) lane1_state = 1;

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-lane-2-state", NULL);
	if(tmp) lane2_state = 1;

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-lane-3-state", NULL);
	if(tmp) lane3_state = 1;

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-lane-map", NULL);
	if(strcmp(tmp->data, "lane_map_0123") == 0)
		dsi_lanemap = 0;
	else
		exit(1);

	dsi_lanes = lane0_state + lane1_state + lane2_state + lane3_state;
printf("static struct lane_configuration %s_lane_config = {\n"
"		    %d, %d, %d, %d, %d, %d\n"
"};\n", panel, dsi_lanes, dsi_lanemap, lane0_state, lane1_state, lane2_state, lane3_state);
}

void dump_timing_info(void *fdt, int id, const char *panel) {
	const struct fdt_property *tmp = NULL;
    uint8_t dsi_mdp_trigger;                                                                                            
    uint8_t dsi_dma_trigger;                                                                                            
    uint8_t tclk_post;                                                                                                  
    uint8_t tclk_pre;

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-t-clk-post", NULL);
	tclk_post = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-t-clk-pre", NULL);
	tclk_pre = fdt32_to_cpu( *(uint32_t*)tmp->data);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-dma-trigger", NULL);
	if(strcmp(tmp->data, "trigger_sw") == 0)
		dsi_dma_trigger = 4;
	else
		exit(1);

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-mdp-trigger", NULL);
	if(strcmp(tmp->data, "none") == 0)
		dsi_mdp_trigger = 0;
	else
		exit(1);

printf("static struct panel_timing %s_timing_info = {\n"
"        %d, %d, 0x%02x, 0x%02x\n"
"};\n", panel,
	dsi_mdp_trigger, dsi_dma_trigger, tclk_post, tclk_pre);
}

void dump_timings(void *fdt, int id, const char *panel) {
	const struct fdt_property *tmp = NULL;
uint8_t timing[12];

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-panel-timings", NULL);
	memcpy(timing, tmp->data, 12);

printf("static struct panel_timing %s_timings[] = {\n"
"       0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n"
"};\n", panel,
	timing[0], timing[1], timing[2],
	timing[3], timing[4], timing[5],
	timing[6], timing[7], timing[8],
	timing[9], timing[10], timing[11]);
}

void dump_reset(void *fdt, int id, const char *panel) {
	const struct fdt_property *tmp = NULL;
    uint8_t pin_state[3] = { 0,0,0};
    uint32_t sleep[3] = { 0,0,0};
    uint8_t pin_direction;

	tmp = fdt_get_property(fdt, id, "qcom,mdss-dsi-reset-sequence", NULL);
pin_direction = 2; //Unused?
for(int i=0; i<3; i++) {
	pin_state[i] = fdt32_to_cpu( ((uint32_t*)tmp->data)[2*i]);
	sleep[i] = fdt32_to_cpu( ((uint32_t*)tmp->data)[2*i+1]);
}


printf("static struct panel_reset_sequence %s_reset_seq = {\n"
"    {%d, %d, %d, }, {%d, %d, %d, }, 2\n"
"};\n", panel,
	pin_state[0], pin_state[1], pin_state[2],
	sleep[0], sleep[1], sleep[2]);
}

void dump_screen(void *fdt, int id, char *panel) {
	const struct fdt_property *panel_on = fdt_get_property(fdt, id, "qcom,mdss-dsi-on-command", NULL);
	const struct fdt_property *panel_off = fdt_get_property(fdt, id, "qcom,mdss-dsi-off-command", NULL);

	dump_cmds(panel_on, panel, "panel_on", 0);
	dump_cmds(panel_off, panel, "panel_off", 1);
	dump_resolution(fdt, id, panel);
	dump_colorinfo(fdt, id, panel);
	dump_state(fdt, id, panel);
	dump_panelinfo(fdt, id, panel);
	dump_lane(fdt, id, panel);
	dump_timing_info(fdt, id, panel);
	dump_timings(fdt, id, panel);
	dump_reset(fdt, id, panel);
	dump_misc(fdt, id, panel);
}

void display_property(void *fdt, int id) {
	int len = 0;
	const struct fdt_property *prop = fdt_get_property_by_offset(fdt, id, &len);
	if(!prop)
		return;
//	print_indent();
	const char *name = fdt_string(fdt, fdt32_to_cpu(prop->nameoff));
//	if(name)
//		printf("%s: '%d:%p'\n", name, fdt32_to_cpu(prop->len), prop->data);
}

void browse_node(void* fdt, int offset) {
	int len = 0;
	const char *name = fdt_get_name(fdt, offset, &len);
//	print_indent();
//	printf("%s {\n", name);
	indent++;

	if(strcmp("qcom,mdss_dsi_ili9881c_haifei_720p_video", name) == 0) {
		dump_screen(fdt, offset, "ili9881c_haifei_720p_video");
	}

	int id = fdt_first_property_offset(fdt, offset);
	while(id>=0) {
		display_property(fdt, id);

		id = fdt_next_property_offset(fdt, id);
	}

	id = fdt_first_subnode(fdt, offset);
	while(id >= 0) {
		browse_node(fdt, id);
		id = fdt_next_subnode(fdt, id);
	}
	indent--;

//	print_indent();
//	printf("}\n");
}

int main(int argc, char **argv) {
	int fd = open("dtb", O_RDONLY);
	size_t s = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	void *fdt = mmap(NULL, s, PROT_READ, MAP_SHARED, fd, 0);
	if(fdt_check_header(fdt))
		return 1;

	browse_node(fdt, 0);
	return 0;
}
