import sys
import re

def parseLine(line):

    vertices = [];

    #strip the line and remove the white spaces
    sp = line.strip().split();

    #input validation
    if len(sp) == 0:
        raise Exception('Command must not be blank.');
        return NULL;
    elif len(sp) == 1:
        if sp[0] =='gg':
            cmd = 4;
            return [cmd, 0, 0];
        else:
            raise Exception('Command provide is invalid')
            return NULL;
    else:
        #extract the command  
        if sp[0] =='add':
            cmd = 1;
        elif sp[0] =='mod':
            cmd = 2;
        elif sp[0] =='rm':
            cmd = 3;
        else:
            raise Exception('Command provided is invalid');
            return NULL
        
        #extract the street name
        pattern = '"[^"]*"';
        regex_result = re.search(pattern, line);
        if(regex_result):
            stname = line[regex_result.span()[0]:regex_result.span()[1]];
        else:
            raise Exception('No valid street name found. Provide proper street name in double quotes')
            return NULL;

        if stname == "\"\"":
            raise Exception('Street name cannot be empty')
            return NULL;   
             
        #lets remove the cmd and street name part from the line
        line = line[regex_result.span()[1]+1:]
        sp = line.strip().split(")");

        #if it's a rm command, make sure there's nothing else left in the line, and then let's exit here, we have everything we need
        if (cmd == 3) :
            if line.replace(" ", "") == '\n' or line.replace(" ", "") == '':
                return [cmd,stname,0]; 
            else:
                raise Exception('Invalid rm command');
                return NULL


        for i in sp:
            pattern_xpos_ypos = '\([0-9]+,[0-9]+\)'; #detects (x,y)
            pattern_xpos_yneg = '\([0-9]+,-[0-9]+\)'; #detects (x,-y)
            pattern_xneg_ypos = '\(-[0-9]+,[0-9]+\)'; #detects (-x,y)
            pattern_xneg_yneg = '\(-[0-9]+,-[0-9]+\)'; #detects (-x,-y)
            
            #strip if any blank spaces are present
            i = i.replace(" ", "")
            i = i + ')'
            if i == ')':
                break;

            #caretestin coordinates must be provided in one of the four above forms, if else raise an exception
            if re.match(pattern_xpos_ypos, i) or re.match(pattern_xpos_yneg, i) or re.match(pattern_xneg_ypos, i) or re.match(pattern_xneg_yneg, i):
                #Find , 
                comma_pos = re.search(',',i).start();
                #Find y coordinate
                x = int(i[1:comma_pos]);
                #Find y coordinate
                y = int(i[comma_pos+1:-1]);

                vertices.append ([float(x),float(y)]);         
            else:
                raise Exception('Invalid Cartesian coordinates. Cartestian coordinates must be provided in (x,y) format');
                return NULL

        if sp[-1].replace(" ","") != '':
            raise Exception('Invalid Cartesian coordinates. Cartestian coordinates must be provided in (x,y) format');
            return NULL

        if (cmd == 1 or cmd == 2) and len(vertices) <2 :
            raise Exception('Atleast two coordinates required for add and mod commands');
            return NULL 

    return [cmd,stname,vertices]; 