<? include ("barry.inc"); ?>

<? createHeader("Troubleshooting"); ?>

<? createSubHeader("Common Issues When Working With Barry"); ?>


<p><b>Q:</b> When I run btool, it hangs for 30 seconds and then times out on
the USB read call.</p>

<p><b>A:</b> To verify that you are experiencing the right issue, run
"btool -vl" to get a verbose USB log.  At the beginning
the USB configuration and endpoint information is printed.
If the last set of endpoints have 0's in the values, you are
experiencing the problem.  The proper set of endpoints will not
be displayed with "lsusb -v" in this case either.</p>

<p>One workaround is to run bcharge twice like this:</p>

<pre>
	bcharge
	bcharge -o
</pre>

<p>After the second bcharge, btool should work normally.</p>


<hr/>


<p><b>Q:</b> I've installed Barry, and bcharge is set to run automatically,
but as soon as I plug my device into the USB port, the device reboots.</p>

<p><b>A:</b>Make sure you're running a recent version of Barry, at least
0.16 or 0.17.</p>

<p>Depending on the age of your kernel (older than 2.6.34?), you may have
the berry_charge kernel module running, in which case you need to choose
which method you wish to use.  Do not run both berry_charge and bcharge.</p>


<hr/>


<p><b>Q:</b> One of the Barry applications crashed, and now I keep getting
a Desktop error message in hex.</p>

<p><b>A:</b> If a Barry application crashes, that means the Desktop mode was
not shutdown properly.  Your device will likely appear to still
be "busy" as well.  Currently the only way to fix this is to
reset the device, either by unplugging/replugging it, or by
running the program "breset".</p>


<hr/>


<p><b>Q:</b> Everytime I restore a backup with the backup GUI, the database
I need does not get restored.  What gives?</p>

<p><b>A:</b> Both the backup and the restore are filtered based on your
configuration.  Check your Edit | Config... settings and make
sure the needed database is turned on in <i>both</i> backup and restore.</p>


<hr/>


<p><b>Q:</b> I ran the backup, but can't find my backup files!
Where did they go?</p>

<p><b>A:</b> The backup program saves all backup files as gzip'd tarballs
under your home directory:  ~/.barry/backup/pin_number/</p>

<p> Each backup session creates a new file, named with the
pin number, date, and time.</p>

