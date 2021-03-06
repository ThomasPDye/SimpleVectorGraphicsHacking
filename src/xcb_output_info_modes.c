#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <inttypes.h>

int
main(int argc, char const *argv[])
{
    /* Open the connection to the X server. Use the DISPLAY environment variable */
    int screenNum;
    xcb_connection_t *connection = xcb_connect (NULL, &screenNum);

    /* Get the first X screen and it's root window */
    const xcb_setup_t *setup = xcb_get_setup (connection);
    xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(setup);

    /* Send a request for screen resources to the X server */
    xcb_randr_get_screen_resources_cookie_t screen_resources_cookie
        = xcb_randr_get_screen_resources(connection,screen_iterator.data->root);
    
    /* Receive reply from X server */
    xcb_randr_get_screen_resources_reply_t *screen_resources_reply
        = xcb_randr_get_screen_resources_reply(connection,screen_resources_cookie,NULL);

    /* Get Number of Outputs and pointer to first Output in array */
    int screen_num_outputs;
    xcb_randr_output_t * screen_outputs;
    if(screen_resources_reply)
    {
        screen_num_outputs = xcb_randr_get_screen_resources_outputs_length(screen_resources_reply);
        screen_outputs = xcb_randr_get_screen_resources_outputs(screen_resources_reply);
    }
    else
    {
        fprintf(stderr,"could not get screen resources reply\n");
        return -1;
    }

    /* Request properties of each output for current screen */
    xcb_randr_get_output_info_cookie_t* output_info_cookie = 
        malloc(sizeof(xcb_randr_get_output_info_cookie_t)*screen_num_outputs);
    for (int o = 0; o < screen_num_outputs; o++)
    {
        output_info_cookie[o] = xcb_randr_get_output_info(connection,screen_outputs[o], 0);
    }

    /* Receive properties of each output for current screen */
    xcb_randr_get_output_info_reply_t** output_info_reply =
        (xcb_randr_get_output_info_reply_t**)
        malloc(sizeof(xcb_randr_get_output_info_reply_t*)*screen_num_outputs);
    for (int o = 0; o < screen_num_outputs; o++)
    {
        output_info_reply[o] = xcb_randr_get_output_info_reply(connection,output_info_cookie[o],NULL);
    }

    /* print out desired info on each output */
    for (int o = 0; o < screen_num_outputs; o++)
    {
        /* Get Output name (and length of name) */
        char * output_name_str;
        uint8_t * output_name = xcb_randr_get_output_info_name(output_info_reply[o]);
        int output_name_length = xcb_randr_get_output_info_name_length(output_info_reply[o]);

        if(output_name[output_name_length-1]!='\0')
        {
            output_name_str = (char*) calloc((output_name_length+1),sizeof(char));
            for (int s = 0; s < output_name_length; s++)
            {
                output_name_str[s] = (char) output_name[s];
            }
            
        }

        /* Get Output mode list (and length of list) */
        int output_modes_length = xcb_randr_get_output_info_modes_length(output_info_reply[o]);
        xcb_randr_mode_t * output_modes = xcb_randr_get_output_info_modes(output_info_reply[o]);

        /* Print out Output name */
        printf("%s\tModes:\n",output_name);
        for (int m = 0; m < output_modes_length; m++)
        {
            printf("\t%X\n",output_modes[m]);
        }
        
    }

    return 0;
}
