/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ski <ski@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/10 14:02:56 by ski               #+#    #+#             */
/*   Updated: 2022/06/10 14:46:39 by ski              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 #include <sys/wait.h>

/* ************************************************************************** */
#define TYPE_NORMAL	0
#define TYPE_PIPE	1
/* ************************************************************************** */
typedef struct s_param
{
	int output_type;
	int stdin_original;
}	t_param;
/* ************************************************************************** */
int ft_strlen(char *str)
{
	int i;

	if (!str)
		return 0;

	i = 0;
	while (str[i])
		i++;

	return i;	
}
/* ************************************************************************** */
void write_to_stderr(char *str)
{
	write(STDERR_FILENO, str, ft_strlen(str));
}
/* ************************************************************************** */
void cd_builtin (char **argv)
{
	int i;

	i = 0;
	while (argv[i])
		i++;

	if (i != 2)
		write_to_stderr("error: cd: bad arguments\n");
	else if (chdir(argv[1]) == -1 )
	{
		write_to_stderr("error: cd: cannot change directory to ");
		write_to_stderr(argv[1]);
		write_to_stderr("\n");
	}
}
/* ************************************************************************** */
void exec_cmd(t_param *p, char **argv, char **envp)
{
	int fk;
	int pipefd[2];
	int status;
	
	
	if (argv[0] == NULL)
		return ;

	if (strcmp(argv[0], "cd") == 0)
	{
		cd_builtin(argv);
		return;
	}

	if (p->output_type == TYPE_PIPE && pipe(pipefd) == -1)
	{
		write_to_stderr("error: fatal\n");
		close(p->stdin_original);
		exit(EXIT_FAILURE);
	}

	fk = fork();

	if (fk == -1)
	{
		write_to_stderr("error: fatal\n");
		close(p->stdin_original);
		exit(EXIT_FAILURE);
	}

	if (fk == 0)
	{
		if (p->output_type == TYPE_PIPE)
		{
			close(pipefd[0]);
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[1]);
		}
		
		execve(argv[0], argv, envp);
		write_to_stderr("error: cannot execute ");
		write_to_stderr(argv[0]);
		write_to_stderr("\n");
		close(p->stdin_original);
		exit(EXIT_FAILURE);
	}
	else
	{
		if (p->output_type == TYPE_PIPE)
		{
			close(pipefd[1]);
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
		}
		else
			dup2(p->stdin_original, STDIN_FILENO);
		waitpid(fk, &status, 0);
	}	
}
/* ************************************************************************** */
int main (int argc, char **argv, char **envp)
{
	t_param p;
	int	i_end;
	int j_start;

	p.output_type = TYPE_NORMAL;
	p.stdin_original = dup(STDIN_FILENO);
	i_end = 1;
	j_start = 1;
	
	while (argv[i_end])
	{
		if (strcmp(argv[i_end], "|") == 0 || strcmp(argv[i_end], ";") == 0)
		{
			if (strcmp(argv[i_end], "|") == 0)
				p.output_type = TYPE_PIPE;
			argv[i_end] = NULL;

			exec_cmd(&p, &argv[j_start], envp);
			p.output_type = TYPE_NORMAL;
			j_start = i_end + 1;			
		}
		else if (argv[i_end + 1] == NULL)
			exec_cmd(&p, &argv[j_start], envp);
		i_end++;
	}
	close(p.stdin_original);
	return 0;
}



/* ************************************************************************** */