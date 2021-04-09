char *basename(f)
char *f;
{
    char *t = rindex(f, '/');
    return t ? t+1 : f;
}