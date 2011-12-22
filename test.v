{.for 1 to 50}
	{.SELECT '@LOOP' as `test_@LOOP`}
{.next}

{.SELECT 5 as r}
{.for 1 to @r}
	{.SELECT '@test_$LOOP' as asdf}
	{.asdf}
{.next}
