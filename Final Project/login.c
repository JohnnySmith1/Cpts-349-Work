char *tty;

#include "ucode.c"

main(int argc, char *argv[])   // invoked by exec("login /dev/ttyxx")
{
  int stdin, stdout, stderr, passwordFileFd, linesCount, i;
  char *token, *lineToken, *temp;
  char username[40], password[40], buf[1024], lines[50][64], execCommand[64], loginWelcomeMessage[64], loginFailure[64];
  char passwdUser[64], passwdPasswd[64], passwdGid[64], passwdUid[64], passwdFullname[64], passwdHomeDir[64], passwdProgram[64];


  // This is /dev/tty0 when logging in first time
  tty = argv[1];

  // initialize lines to 0
  for(i = 0; i < 50; i++)
  {
    strcpy(lines[i], 0);
  }

  passwordFileFd = 0;

  // 1. login process may run on different terms
  close(0);
  close(1);
  close(2);

  // 2. We open its own tty as stdin, stdout, stderr
  // For read
  stdin = open(tty, 0);
  // For write
  stdout = open(tty, 1);
  // For error
  stdout = open(tty, 2);




  // 3. store tty string in PROC.tty[] for putc()
  settty(tty);

  // NOW we can use printf, which calls putc() to our tty
  sprintf(loginWelcomeMessage, "GABE LOGIN : open %s as stdin, stdout, stderr\n", tty);
  gwrite(2, loginWelcomeMessage, strlen(loginWelcomeMessage));

  // ignore Control-C interrupts so that
  // Control-C KILLs other procs on this tty but not the main sh
  signal(2,1);
  
  while(1)
  {
    linesCount = 0;

    // 1. show login:           to stdout
    gwrite(2, "login: ", 7);
    // 2. read user name        from stdin
    gets(username);
    // printf("Entered username: %s\n", username);

    // 3. show passwd
    gwrite(2, "password: ", 10);
    // 4. read user passwd
    gets(password);
    // printf("Entered password: %s\n", password);

    // 5. verify user name and passwd from /etc/passwd file
    passwordFileFd = open("/etc/passwd", 0);

    read(passwordFileFd, buf, 1024);

    // We first read in all the lines
    i = 0;
    token = strtok(buf, "\n");


    while(token != 0)
    {
      strcpy(lines[i], token);
      i++;
      linesCount++;
      token = strtok(0, "\n");
    }

    // We then tokenize each line, one at a time
    i = 0;

    while(i != linesCount)
    {
      lineToken = strtok(lines[i], ":"); // username
      strcpy(passwdUser, lineToken);
      lineToken = strtok(0, ":");  // password
      strcpy(passwdPasswd, lineToken);

      // 6. Check if user account is valid
      if((strcmp(passwdUser, username) == 0) && (strcmp(passwdPasswd, password) == 0))
      {
        // We found a matching user account!
        lineToken = strtok(0, ":"); // gid
        strcpy(passwdGid, lineToken);
        lineToken = strtok(0, ":"); // uid
        strcpy(passwdUid, lineToken);
        lineToken = strtok(0, ":"); // fullname
        strcpy(passwdFullname, lineToken);
        lineToken = strtok(0, ":"); // HOMEDIR
        strcpy(passwdHomeDir, lineToken);
        lineToken = strtok(0, ":"); // program
        strcpy(passwdProgram, lineToken);

        // printf("Setting uid=%d and gid=%d...\n", atoi(passwdUid), atoi(passwdGid));
        // Now we setuid to user uid
        chuid(atoi(passwdUid), atoi(passwdGid));

        // printf("Setting HOME directory to: %s\n", passwdHomeDir);
        // chdir to user HOME directory
        chdir(passwdHomeDir);

        // exec to the program in users's account
        strcat(execCommand, passwdProgram);

        printf("Executing: %s\n", execCommand);
        // Off we go to the shell
        exec(execCommand);

      }

      i++;
    }

    sprintf(loginFailure, "GABE LOGIN FAILED: Login with user %s and password %s.\n", username, password);
    gwrite(2, loginFailure, strlen(loginFailure));

  }

  // gwrite(2, "login somehow is exiting!\n", 26);
  exit(0);
}