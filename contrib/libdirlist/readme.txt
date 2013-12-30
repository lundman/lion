
This is a helper library shipped with lib lion. It is to help a
nonblocking application perform directory listings without stalling.

The idea is that the library forks off a child/ren that are worker
slaves to look up the information. The master process then schedules a
directory to be listed and the listing information is eventually
returned.

The worker slave(s) queues all the requests and processes one at a
time in fifo order. 

Master to slaves:

<id> <flags> /path

Change! We also need to encode in the precat path, so now we actually
send

<id> <flags> <index> /precat /path

Where index is how many bytes precat is (can be 0). We have a space in
there so we can easily null-terminate.


Reply:
 <id> /path

-or-
 <id> <errno> reason

-or-
 <id> start
 ...
 <id> stop



(this/next is not used)
-or-
 <XML start><id>id</id> ?
  ...
 </xml>


Suggested options are:

These are groups in Mutually Exclusive options:

* -l  Produce list in "ls -l" format (classic FTP)
  -1  Produce list in "ls -1" format (only name, one per line)
  -X  Produce list in XLM format. (MLST)

* -T  Create a temporary file and return its name.
  -P  Return data over the pipe connection.

[*] Default, and used if not supplied.


These are optional modifiers to previous options.
  -t  Sort list in date order
  -N  Sort list in name order
  -r  Reverse sorting order
  -D  Sort directories before files. (Like Windows)
  -s  Sort list in size order
  -I  Case insensitive sorting. (Like Windows)
  -a  Show dot-files
  -W  Show directory content size instead of block count (expensive)
  -G  If found ".genre" file, display this instead of group field.




Although, sorting don't make much difference with XML format. Default
is to return it unsorted (unlike "ls -l") as it is more
efficient. Many clients sort it locally anyway.









                   dirlist_init( int numchildren) 

Initialise the dirlist library, and start the children.


                   dirlist_free()

Release the children, and the library.


                   dirlist_list(lion_t handle, int flags, char path, user_data)

Request a new directory to be listed.
"handle" is optional, it serves only as a guide to where you want the
events being received as. If NULL events go to lion_user_input() as
per normal. user_data is passed along as per usual.



					int dirlist_a2f(char *str);

Take a string, like "-ltPa" and produce an integer holding the flag
equivalent. 
(That is, DIRLIST_LONG | DIRLIST_SORT_DATE | DIRLIST_PIPE | DIRLIST_SHOW_DOT, 
in this example.)





Results returned in LION_INPUT.

Client will get:

o If requested as an external file
  ":0 /tmp/filename.txt"      0 is success.
  ":errno The string error"   for failure, like ":13 Permission denied".

o If requested in the pipe
  ":0 -list follows"          0 is success, followed eventually by a
  ":END"

  ":errno The string error"   for failure, like ":13 Permission denied".






TODO:

There is no reverse of dirlist_a2f(), should probably implement dirlist_f2a
for completeness sake.

libdirlist produces incorrect output for any special device entry. It
should look like something like:

brw-r-----  1 root     operator   16,      0 Sep  8 02:54 ccd0a

but looks currently like:
brw-r-----  1 root     operator           0 Sep  8 02:54 ccd0a

ie, it omits to print the "high and low" device numbers.

BUGS:

Currently, if you start up a directory listing request, and you pass
it the "handle" of another lion type, say a remote connection, and it
is closed, or lost, the libdir does NOT know this, and will keep
accessing lion_get_handle(handle). IE, read free()ed memory. BAD!

So, two solutions. One, leave it as is, but, provide API where the
application can call dirlist_abort() on those events.

and/or

When calling dirlist_list, change the handle's handler function to our
own, then simply relay all events, but noticing any of the close or
lost events to internally clean up.

The second is preferred, the only issue is if a user calls set_handler
afterwards, it would go back to the problem. However, this can be a
restriction. They can change the handler, they just have to do before
they call dirlist.


Currently, if you list following symlinks, it does not detect cyclic
graphs, and would go on forever. I have a static maximum depth at 20,
but this needs to be adjustable. Would be nice to also be able to
detect cyclic links.



Actually, if you set a new "handler" function for the duration that a
handle is expecting dirlist result, you can deal with the issues of
CLOSED and LOST properly.




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In addition, we need API to pre-load a set of names for it to ignore
in directory listings. Like ".journal" for linux users, ".state-*" for
internal LundFTPD and so on. This will be a fnmatch set of
strings. These should probably be called before the init call.

dirlist_hide_file("*.hideme");


Additionally, the uid and gid lookup calls need to be allowed to be
external if that is so wished.

		 dirlist_set_uidlookup( login_uid_to_name_lookup_function );
		 dirlist_set_gidlookup( login_gid_to_name_lookup_function );
		 char *user_uidgid_to_name_function(int uid);

Where the return value says:

	 !0 - look up successful, strings returned
	  0 - look up failed, use system uid2name calls.

Note if you fail to look up the uid & gid, and don't want dirlist to
use the system function to look up the names, you can just return
string versions of uid and gid and return successful.

WARNING! The uid and gid functions are used in callback fashion from A
DIFFERENT FORKED PROCESS. Be aware of this in your implementation.
~~~~~~~~~~~~~~~~~~~~~~~~

The string returned by the callback function can be static and is not
free()d by dirlist. (But rather duplicated internally so the same
static buffer can be re-used).


Finally, we need permission checks on whether a certain user can enter
a particular directory for listing. This is a callback, but it is used
as the forked process (be aware of this in your code) so it is ok for
it to be called so often.

dirlist_set_directory_callback( user_list_directory );
int user_list_directory( char *directory, void *user_data );

   0 - user may list this directory
  !0 - user may not list this directory, where x is errno to display.


Recursing thing wouldn't be so bad if we actually HAD user_data. But
we do not. This makes it rather complicated. Since the users function
would need to know in what CONTEXT the directory permission is to be
decided. Just receiving "/dir/path/funk/" in the function is rather
useless.

Let's stop them recursing for now.


